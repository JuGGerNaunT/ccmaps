﻿using System.IO;
using CNCMaps.VirtualFileSystem;
using System.Runtime.InteropServices;
using System.Collections.Generic;
using CNCMaps.Utility;
using CNCMaps.Encodings;
using System;
using CNCMaps.MapLogic;

namespace CNCMaps.FileFormats {

	class ShpFile : VirtualFile {

		[StructLayout(LayoutKind.Sequential, Pack = 1)]
		public struct ShpFileHeader {
			private short zero;
			public short cx;
			public short cy;
			public short c_images;
		}

		[StructLayout(LayoutKind.Sequential, Pack = 1)]
		struct ShpImageHeader {
			public short x;
			public short y;
			public short cx;
			public short cy;
			public int compression;
			private int unknown;
			private int zero;
			public int offset;
		}

		struct ShpImage {
			public ShpImageHeader header;
			public byte[] imageData;
		}

		ShpFileHeader fileHeader;
		List<ShpImage> images;

		bool initialized = false;

		public ShpFile(Stream baseStream, string filename, int baseOffset, int fileSize, bool isBuffered = true)
			: base(baseStream, filename, baseOffset, fileSize, isBuffered) {
		}

		public void Initialize() {
			initialized = true;
			this.fileHeader = EzMarshal.ByteArrayToStructure<ShpFileHeader>(Read(Marshal.SizeOf(typeof(ShpFileHeader))));
			images = new List<ShpImage>(this.fileHeader.c_images);
			int prevOffset = int.MinValue;
			for (int i = 0; i < this.fileHeader.c_images; i++) {
				ShpImage img = new ShpImage();
				img.header = EzMarshal.ByteArrayToStructure<ShpImageHeader>(Read(Marshal.SizeOf(typeof(ShpImageHeader))));
				images.Add(img);

				// if this is a valid image, make sure the offsets are contiguous
				if (img.header.cx * img.header.cy > 0) {
					System.Diagnostics.Debug.Assert(prevOffset < img.header.offset);
					prevOffset = img.header.offset;
				}
			}
		}

		private ShpImage GetImage(int imageIndex) {
			ShpImage img = images[imageIndex];
			// make sure imageData is present/decoded if needed
			if (img.imageData == null) {
				Position = img.header.offset;
				int c_px = img.header.cx * img.header.cy;

				if ((img.header.compression & 2) == 2) {
					img.imageData = new byte[c_px];
					int compressedEnd = (int)this.Length;
					if (imageIndex < images.Count - 1)
						compressedEnd = images[imageIndex + 1].header.offset;
					if (compressedEnd < img.header.offset)
						compressedEnd = (int)this.Length;
					Format3.DecodeInto(Read(compressedEnd - img.header.offset), img.imageData, img.header.cx, img.header.cy);
				}
				else {
					img.imageData = Read(c_px);
				}
			}
			return img;
		}

		unsafe public void Draw(int frameIndex, DrawingSurface ds, int x_offset, int y_offset, short height, Palette p) {
			if (!initialized) Initialize();

			var image = GetImage(frameIndex);
			var h = image.header;
			uint c_px = (uint)(h.cx * h.cy);
			int stride = ds.bmd.Stride;
			var zBuf = ds.GetZBuffer();

			if (c_px <= 0 || h.cx < 0 || h.cy < 0 || frameIndex > fileHeader.c_images)
				return;

			byte* w_low = (byte*)ds.bmd.Scan0;
			byte* w_high = (byte*)ds.bmd.Scan0 + stride * ds.bmd.Height;

			int dx = x_offset + 30 - fileHeader.cx / 2 + h.x,
				dy = y_offset - fileHeader.cy / 2 + h.y;
			byte* w = (byte*)ds.bmd.Scan0 + dx * 3 + stride * dy;
			int zIdx = dx + dy * ds.Width;
			int rIdx = 0;

			for (int y = 0; y < h.cy; y++) {
				for (int x = 0; x < h.cx; x++) {
					byte paletteValue = image.imageData[rIdx];
					if (paletteValue != 0 && w_low <= w && w < w_high) {
						*(w + 0) = p.colors[paletteValue].B;
						*(w + 1) = p.colors[paletteValue].G;
						*(w + 2) = p.colors[paletteValue].R;
						zBuf[zIdx] = Math.Max(zBuf[zIdx], height);
					}
					// Up to the next pixel
					rIdx++;
					w += 3;
				}
				w += stride - 3 * h.cx;	// ... and if we're no more on the same row,
				// adjust the writing pointer accordingy
			}
		}

		unsafe public void DrawShadow(int frameIndex, DrawingSurface ds, int x_offset, int y_offset) {
			if (frameIndex >= images.Count / 2) return;
			var image = GetImage(frameIndex + images.Count / 2);
			var h = image.header;
			uint c_px = (uint)(h.cx * h.cy);
			int stride = ds.bmd.Stride;
			var shadows = ds.GetShadows();

			if (c_px <= 0 || h.cx < 0 || h.cy < 0 || frameIndex > fileHeader.c_images)
				return;

			byte* w_low = (byte*)ds.bmd.Scan0;
			byte* w_high = (byte*)ds.bmd.Scan0 + stride * ds.bmd.Height;

			int dx = x_offset + 30 - fileHeader.cx / 2 + h.x,
				dy = y_offset - fileHeader.cy / 2 + h.y;
			byte* w = (byte*)ds.bmd.Scan0 + dx * 3 + stride * dy;
			int zIdx = dx + dy * ds.Width;
			int rIdx = 0;

			for (int y = 0; y < h.cy; y++) {
				for (int x = 0; x < h.cx; x++) {
					if (image.imageData[rIdx] != 0 && shadows[zIdx] == false && w_low <= w && w < w_high) {
						*(w + 0) /= 2;
						*(w + 1) /= 2;
						*(w + 2) /= 2;
						shadows[zIdx] = true;
					}
					// Up to the next pixel
					rIdx++;
					zIdx++;
					w += 3;
				}
				w += stride - 3 * h.cx;	// ... and if we're no more on the same row,
				zIdx += ds.Width - h.cx;
				// adjust the writing pointer accordingy
			}
		}

		public unsafe void DrawAlpha(int frameIndex, DrawingSurface ds, int xOffset, int yOffset) {
			var image = GetImage(frameIndex + images.Count / 2);
			var h = image.header;
			uint c_px = (uint)(h.cx * h.cy);
			int stride = ds.bmd.Stride;
			
			if (c_px <= 0 || h.cx < 0 || h.cy < 0 || frameIndex > fileHeader.c_images)
				return;

			byte* w_low = (byte*)ds.bmd.Scan0;
			byte* w_high = (byte*)ds.bmd.Scan0 + stride * ds.bmd.Height;

			int dx = xOffset + 30 - fileHeader.cx / 2 + h.x,
				dy = yOffset - fileHeader.cy / 2 + h.y;
			byte* w = (byte*)ds.bmd.Scan0 + dx * 3 + stride * dy;

			int rIdx = 0;

			for (int y = 0; y < h.cy; y++) {
				for (int x = 0; x < h.cx; x++) {
					if (image.imageData[rIdx] != 0 && w_low <= w && w < w_high) {
						float mult = image.imageData[rIdx] / 128.0f;
						*(w + 0) = limit(mult, *(w + 0));
						*(w + 1) = limit(mult, *(w + 0));
						*(w + 2) = limit(mult, *(w + 0));
					}
					// Up to the next pixel
					rIdx++;
					w += 3;
				}
				w += stride - 3 * h.cx;	// ... and if we're no more on the same row,
				// adjust the writing pointer accordingy
			}
		}

		private byte limit(float mult, byte p) {
			return (byte)Math.Max(0f, Math.Min(255f, mult));
		}
	}
}