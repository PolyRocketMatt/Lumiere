using UnityEngine;
using System.Runtime.InteropServices;

// ReSharper disable once CheckNamespace
public static class BufferUtils
{
    public static void ConstructStructuredComputeBuffer<T>(ref ComputeBuffer buffer, T[] data) where T : struct
    {
        //  Make sure the length of the data is not zero
        var bufferLength = Mathf.Max(1, data.Length);
        var bufferStride = Marshal.SizeOf(typeof(T));

        //  If the buffer is null, wrong size, ... we create a new one
        if (buffer == null || !buffer.IsValid() || buffer.count != bufferLength || buffer.stride != bufferStride)
        {
            buffer?.Release();
            buffer = new ComputeBuffer(bufferLength, bufferStride, ComputeBufferType.Structured);
        }

        buffer.SetData(data);
    }
}