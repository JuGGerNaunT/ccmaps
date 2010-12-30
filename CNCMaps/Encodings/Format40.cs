using System.IO;
using CNCMaps.VirtualFileSystem;

namespace CNCMaps.Encodings {

	public static class Format40 {

		public static int DecodeInto(byte[] src, byte[] dest) {
			MemoryStream ms = new MemoryStream(src);
			var ctx = new VirtualFile(ms, 0, src.Length);

			int destIndex = 0;

			while (true) {
				byte i = ctx.ReadByte();
				if ((i & 0x80) == 0) {
					int count = i & 0x7F;
					if (count == 0) {
						// case 6
						count = ctx.ReadByte();
						byte value = ctx.ReadByte();
						for (int end = destIndex + count; destIndex < end; destIndex++)
							dest[destIndex] ^= value;
					}
					else {
						// case 5
						for (int end = destIndex + count; destIndex < end; destIndex++)
							dest[destIndex] ^= ctx.ReadByte();
					}
				}
				else {
					int count = i & 0x7F;
					if (count == 0) {
						count = ctx.ReadInt16();
						if (count == 0)
							return destIndex;

						if ((count & 0x8000) == 0) {
							// case 2, skip
							destIndex += (count & 0x7FFF);
						}
						else if ((count & 0x4000) == 0) {
							// case 3
							for (int end = destIndex + (count & 0x3FFF); destIndex < end; destIndex++)
								dest[destIndex] ^= ctx.ReadByte();
						}
						else {
							// case 4
							byte value = ctx.ReadByte();
							for (int end = destIndex + (count & 0x3FFF); destIndex < end; destIndex++)
								dest[destIndex] ^= value;
						}
					}
					else {
						// case 1
						destIndex += count;
					}
				}
			}
		}

		public unsafe static int DecodeInto(byte* src, int srcLen, byte[] dest) {
			int destIndex = 0;

			while (true) {
				byte i = *src++;
				if ((i & 0x80) == 0) {
					int count = i & 0x7F;
					if (count == 0) {
						// case 6
						count = *src++;
						byte value = *src++;
						for (int end = destIndex + count; destIndex < end; destIndex++)
							dest[destIndex] ^= value;
					}
					else {
						// case 5
						for (int end = destIndex + count; destIndex < end; destIndex++)
							dest[destIndex] ^= *src++;
					}
				}
				else {
					int count = i & 0x7F;
					if (count == 0) {
						count = (*src << 8) + *(src + 1); // read int16 todo, check
						src += 2;
						if (count == 0)
							return destIndex;

						if ((count & 0x8000) == 0) {
							// case 2
							destIndex += (count & 0x7FFF);
						}
						else if ((count & 0x4000) == 0) {
							// case 3
							for (int end = destIndex + (count & 0x3FFF); destIndex < end; destIndex++)
								dest[destIndex] ^= *src++;
						}
						else {
							// case 4
							byte value = *src++;
							for (int end = destIndex + (count & 0x3FFF); destIndex < end; destIndex++)
								dest[destIndex] ^= value;
						}
					}
					else {
						// case 1
						destIndex += count;
					}
				}
			}
		}
	}
}