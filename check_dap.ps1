# check_dap.ps1 - CMSIS-DAP probe health check (read-only, sends no destructive commands)
#
# Usage:  Invoke-Expression (Get-Content -Raw .\check_dap.ps1)
#     or: powershell -ExecutionPolicy Bypass -File .\check_dap.ps1
#
# Reports whether the attached CMSIS-DAP probe actually answers DAP commands.
# A healthy probe returns a non-zero firmware-version string here.
# If responses merely echo back the bytes written, the probe is not running its
# command engine and needs a full power cycle.

$ErrorActionPreference = 'Continue'

Add-Type -TypeDefinition @'
using System;
using System.Runtime.InteropServices;
using System.Text;

public static class DapCheck
{
    [StructLayout(LayoutKind.Sequential)]
    public struct SP_DEVICE_INTERFACE_DATA
    { public int cbSize; public Guid InterfaceClassGuid; public int Flags; public IntPtr Reserved; }

    [StructLayout(LayoutKind.Sequential)]
    public struct HIDD_ATTRIBUTES
    { public int Size; public ushort VendorID; public ushort ProductID; public ushort VersionNumber; }

    [DllImport("setupapi.dll", CharSet = CharSet.Unicode, SetLastError = true)]
    public static extern IntPtr SetupDiGetClassDevs(ref Guid g, IntPtr e, IntPtr h, int f);
    [DllImport("setupapi.dll", CharSet = CharSet.Unicode, SetLastError = true)]
    public static extern bool SetupDiEnumDeviceInterfaces(IntPtr s, IntPtr d, ref Guid g, int i, ref SP_DEVICE_INTERFACE_DATA did);
    [DllImport("setupapi.dll", CharSet = CharSet.Unicode, SetLastError = true)]
    public static extern bool SetupDiGetDeviceInterfaceDetail(IntPtr s, ref SP_DEVICE_INTERFACE_DATA did, IntPtr b, int sz, out int req, IntPtr d2);
    [DllImport("setupapi.dll", SetLastError = true)]
    public static extern bool SetupDiDestroyDeviceInfoList(IntPtr s);
    [DllImport("hid.dll", SetLastError = true)]
    public static extern void HidD_GetHidGuid(out Guid g);
    [DllImport("hid.dll", SetLastError = true)]
    public static extern bool HidD_GetAttributes(IntPtr h, ref HIDD_ATTRIBUTES a);
    [DllImport("hid.dll", SetLastError = true)]
    public static extern bool HidD_GetFeature(IntPtr h, byte[] b, int len);
    [DllImport("kernel32.dll", CharSet = CharSet.Unicode, SetLastError = true)]
    public static extern IntPtr CreateFile(string n, uint a, uint s, IntPtr sa, uint c, uint f, IntPtr t);
    [DllImport("kernel32.dll", SetLastError = true)]
    public static extern bool CloseHandle(IntPtr h);

    public static string FindDapHidPath()
    {
        Guid g; HidD_GetHidGuid(out g);
        IntPtr s = SetupDiGetClassDevs(ref g, IntPtr.Zero, IntPtr.Zero, 0x02 | 0x10);
        if (s == new IntPtr(-1)) return null;
        try
        {
            for (int i = 0; ; i++)
            {
                var did = new SP_DEVICE_INTERFACE_DATA();
                did.cbSize = Marshal.SizeOf(typeof(SP_DEVICE_INTERFACE_DATA));
                if (!SetupDiEnumDeviceInterfaces(s, IntPtr.Zero, ref g, i, ref did)) break;
                int req = 0;
                SetupDiGetDeviceInterfaceDetail(s, ref did, IntPtr.Zero, 0, out req, IntPtr.Zero);
                IntPtr b = Marshal.AllocHGlobal(req);
                string path = null;
                try
                {
                    Marshal.WriteInt32(b, IntPtr.Size == 8 ? 8 : 6);
                    if (SetupDiGetDeviceInterfaceDetail(s, ref did, b, req, out req, IntPtr.Zero))
                        path = Marshal.PtrToStringUni(new IntPtr(b.ToInt64() + 4));
                }
                finally { Marshal.FreeHGlobal(b); }
                if (path == null) continue;
                IntPtr h = CreateFile(path, 0, 3, IntPtr.Zero, 3, 0, IntPtr.Zero);
                if (h == new IntPtr(-1)) continue;
                try
                {
                    var a = new HIDD_ATTRIBUTES();
                    a.Size = Marshal.SizeOf(typeof(HIDD_ATTRIBUTES));
                    if (HidD_GetAttributes(h, ref a) && a.VendorID == 0xFAED) return path;
                }
                finally { CloseHandle(h); }
            }
        }
        finally { SetupDiDestroyDeviceInfoList(s); }
        return null;
    }
}
'@

$path = [DapCheck]::FindDapHidPath()
if (-not $path) {
    Write-Host "FAIL: no CMSIS-DAP (VID 0xFAED) HID interface found - probe not attached." -ForegroundColor Red
    exit 1
}
Write-Host "Probe HID interface: $path"

$h = [DapCheck]::CreateFile($path, [uint32]2147483648, [uint32]3, [IntPtr]::Zero, [uint32]3, [uint32]0, [IntPtr]::Zero)
if ($h -eq [IntPtr]::new(-1)) {
    Write-Host "FAIL: cannot open probe (win32 error $([System.Runtime.InteropServices.Marshal]::GetLastWin32Error()))" -ForegroundColor Red
    exit 1
}

try {
    # DAP_Info, id 0x04 = DAPLink firmware version
    $rep = New-Object byte[] 65
    $rep[0] = 0x00
    $rep[1] = 0x00
    $rep[2] = 0x04
    $sw = [System.Diagnostics.Stopwatch]::StartNew()
    $ok = [DapCheck]::HidD_GetFeature($h, $rep, 65)
    $sw.Stop()

    if (-not $ok) {
        Write-Host "FAIL: DAP_Info request failed (win32 error $([System.Runtime.InteropServices.Marshal]::GetLastWin32Error()))" -ForegroundColor Red
        exit 1
    }

    $len = [int]$rep[2]
    Write-Host ("Reply: echo=0x{0:X2} len={1} in {2} ms" -f $rep[1], $len, $sw.ElapsedMilliseconds)

    if ($len -eq 0) {
        Write-Host "FAIL: probe returned an empty response - command engine is not answering." -ForegroundColor Red
        Write-Host "      Responses that merely repeat the bytes sent (loopback) mean the probe needs a full power cycle."
        exit 1
    }

    $payload = $rep[3..(3 + $len - 1)]
    $text = [System.Text.Encoding]::ASCII.GetString([byte[]]$payload)
    Write-Host ("Payload: " + (($payload | ForEach-Object { '{0:X2}' -f $_ }) -join ' '))

    # A dead command engine answers with what the host sent, or with all-zero padding.
    $loopback = ($payload.Count -ge 2 -and $payload[0] -eq 0x00 -and $payload[1] -eq 0x04)
    $allZero = -not ($payload | Where-Object { $_ -ne 0 })
    if ($loopback -or $allZero) {
        Write-Host "FAIL: probe echoed the request / returned zeros - no real DAP_Info response." -ForegroundColor Red
        Write-Host "      The USB connection is up but the probe's command engine is not running." -ForegroundColor Red
        Write-Host "      Fix: fully power-cycle the probe (unplug USB and any external supply), then retry." -ForegroundColor Yellow
        exit 1
    }
    if ($text.Trim([char]0, ' ') -match '^\d') {
        Write-Host "PASS: probe answered with firmware version '$($text.Trim([char]0))'" -ForegroundColor Green
        exit 0
    }
    Write-Host "WARN: got a response but it does not look like a firmware version string." -ForegroundColor Yellow
    exit 0
}
finally { [void][DapCheck]::CloseHandle($h) }
