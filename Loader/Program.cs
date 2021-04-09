using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace Loader
{
    class Program
    {
        static IntPtr setLibrary(string dll, string function)
        {
            IntPtr h = NativeApi.GetModuleHandle(dll);
            return NativeApi.GetProcAddress(h, function);
        }


        static void injectDLL(int pid, string payloadPath)
        {

            IntPtr processPtr = NativeApi.OpenProcess(NativeApi.PROCESS_ALL_ACCESS, false, pid);

            //getting the pointer to LoadLibraryA in kernel32.dll
            IntPtr loadLibraryPtr = NativeApi.GetProcAddress(NativeApi.GetModuleHandle("kernel32.dll"), "LoadLibraryA");

            //allocate payload path
            IntPtr allocMemAddress = NativeApi.VirtualAllocEx(processPtr, IntPtr.Zero, (uint)((payloadPath.Length + 1) * Marshal.SizeOf(typeof(char))), NativeApi.MEM_COMMIT | NativeApi.MEM_RESERVE, NativeApi.PAGE_READWRITE);

            //write to process memory
            UIntPtr bytesWritten;
            NativeApi.WriteProcessMemory(processPtr, allocMemAddress, Encoding.Default.GetBytes(payloadPath), (uint)((payloadPath.Length + 1) * Marshal.SizeOf(typeof(char))), out bytesWritten);

            //create thread in process, execute loadlibrary and call the allocated path
            NativeApi.CreateRemoteThread(processPtr, IntPtr.Zero, 0, loadLibraryPtr, allocMemAddress, 0, IntPtr.Zero);

        }

        static void Main(string[] args)
        {
            
            if (args.Length < 2)
            {

                Console.WriteLine("Execute managed code in unmanaged code:");
                Console.WriteLine("<Payload Path> <Process Name>");
                Console.WriteLine("Example:");
                Console.WriteLine("C:\\payload.dll notepad");
                Console.WriteLine("By Proxytype - https://github.com/proxytype");
                return;

            }

            string payload = args[0];
            string injectTo = args[1];


            Process[] ps = Process.GetProcessesByName(injectTo);

            if (ps.Length != 0)
            {

                injectDLL(ps[0].Id, payload);
                Console.ReadKey();

            }
        }
    }
}
