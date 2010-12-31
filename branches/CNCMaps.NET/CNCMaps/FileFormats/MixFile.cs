﻿using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using CNCMaps.Encodings;
using CNCMaps.Encodings.FileFormats;
using CNCMaps.VirtualFileSystem;

namespace CNCMaps.FileFormats {

	class MixFile : VirtualFile, IArchive {
		Dictionary<uint, MixEntry> index;
		bool isRmix, isEncrypted;
		long dataStart;

		public MixFile(Stream baseStream, string filename = "",  bool isBuffered = false) : this(baseStream, filename, 0, baseStream.Length, isBuffered) { }

		public MixFile(Stream baseStream, string filename, int baseOffset, long fileSize, bool isBuffered = false)
			: base(baseStream, filename, baseOffset, fileSize, isBuffered) {
			ParseHeader();
		}

		public bool ContainsFile(string filename) {
			return index.ContainsKey(MixEntry.HashFilename(filename));
		}

		private void ParseHeader() {
			Position = 0;
			BinaryReader reader = new BinaryReader(this);
			uint signature = reader.ReadUInt32();

			isRmix = 0 == (signature & ~(uint)(MixFileFlags.Checksum | MixFileFlags.Encrypted));

			if (isRmix) {
				isEncrypted = 0 != (signature & (uint)MixFileFlags.Encrypted);
				if (isEncrypted) {
					index = ParseRaHeader(this, out dataStart).ToDictionary(x => x.Hash);
					return;
				}
			}
			else
				Seek(0, SeekOrigin.Begin);

			isEncrypted = false;
			index = ParseTdHeader(this, out dataStart).ToDictionary(x => x.Hash);
		}

		const long headerStart = 84;

		List<MixEntry> ParseRaHeader(VirtualFile reader, out long dataStart) {
			//BinaryReader reader = new BinaryReader(s);
			byte[] keyblock = reader.Read(80);
			byte[] blowfishKey = new BlowfishKeyProvider().DecryptKey(keyblock);

			uint[] h = ReadUints(reader, 2);

			Blowfish fish = new Blowfish(blowfishKey);
			MemoryStream ms = Decrypt(h, fish);
			BinaryReader reader2 = new BinaryReader(ms);

			ushort numFiles = reader2.ReadUInt16();
			reader2.ReadUInt32(); /*datasize*/

			reader.Position = headerStart;

			int byteCount = 6 + numFiles * MixEntry.Size;
			h = ReadUints(reader, (byteCount + 3) / 4);

			ms = Decrypt(h, fish);

			dataStart = headerStart + byteCount + ((~byteCount + 1) & 7);

			long ds;
			return ParseTdHeader(new VirtualFile(ms), out ds);
		}

		static MemoryStream Decrypt(uint[] h, Blowfish fish) {
			uint[] decrypted = fish.Decrypt(h);

			MemoryStream ms = new MemoryStream();
			BinaryWriter writer = new BinaryWriter(ms);
			foreach (uint t in decrypted)
				writer.Write(t);
			writer.Flush();

			ms.Position = 0;
			return ms;
		}

		uint[] ReadUints(VirtualFile r, int count) {
			uint[] ret = new uint[count];
			for (int i = 0; i < ret.Length; i++)
				ret[i] = r.ReadUInt32();

			return ret;
		}

		List<MixEntry> ParseTdHeader(VirtualFile s, out long dataStart) {
			List<MixEntry> items = new List<MixEntry>();

			BinaryReader reader = new BinaryReader(s);
			ushort numFiles = reader.ReadUInt16();
			/*uint dataSize = */
			reader.ReadUInt32();

			for (int i = 0; i < numFiles; i++)
				items.Add(new MixEntry(reader.ReadUInt32(), reader.ReadUInt32(), reader.ReadUInt32()));

			dataStart = s.Position;
			return items;
		}

		class MixEntry {
			public readonly uint Hash;
			public readonly uint Offset;
			public readonly uint Length;

			public MixEntry(uint hash, uint offset, uint length) {
				Hash = hash;
				Offset = offset;
				Length = length;
			}

			public void Write(BinaryWriter w) {
				w.Write(Hash);
				w.Write(Offset);
				w.Write(Length);
			}

			public override string ToString() {
				string filename;
				if (Names.TryGetValue(Hash, out filename))
					return string.Format("{0} - offset 0x{1:x8} - length 0x{2:x8}", filename, Offset, Length);
				else
					return string.Format("0x{0:x8} - offset 0x{1:x8} - length 0x{2:x8}", Hash, Offset, Length);
			}

			public static uint HashFilename(string filename) {
				if (filename.Length > 12)
					filename = filename.Substring(0, 12);
				filename = filename.ToUpperInvariant();
				int l = filename.Length;
				int a = l >> 2;
				if ((l & 3) != 0) {
					filename += (char)(l - (a << 2));
					int i = 3 - (l & 3);
					while (i-- != 0) filename += filename[a << 2];
				}
				return CNCMaps.Encodings.CRC32.CalculateCrc(Encoding.ASCII.GetBytes(filename));
			}

			static Dictionary<uint, string> Names = new Dictionary<uint, string>();

			public static void AddStandardName(string s) {
				uint hash = HashFilename(s);
				Names.Add(hash, s);
			}

			public const int Size = 12;
		}

		public VirtualFile OpenFile(string filename, FileFormat f = FileFormat.None) {
			MixEntry e;
			if (!index.TryGetValue(MixEntry.HashFilename(filename), out e))
				return null;
			else
				return FormatHelper.OpenAsFormat(baseStream, filename, (int)(baseOffset + dataStart + e.Offset), (int)e.Length, f);
		}

		public IEnumerable<uint> AllFileHashes() {
			return index.Keys;
		}

		[Flags]
		enum MixFileFlags : uint {
			Checksum = 0x10000,
			Encrypted = 0x20000,
		}
	}
}