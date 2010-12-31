﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace CNCMaps.Encodings {
	class Format3 {
		public unsafe static uint DecodeInto(byte[] src, byte[] dst, int cx, int cy) {
			fixed (byte* pSrc =  src, pDst = dst) {
				return DecodeInto(pSrc, src.Length, pDst, cx, cy);
			}
		}

		public unsafe static uint DecodeInto(byte* src, int srcLen, byte* dst, int cx, int cy) {
			byte* r = src;
			byte* w = dst;
			for (int y = 0; y < cy; y++) {
				ushort count = (ushort)(*(ushort*)r - 2);
				r += 2;
				int x = 0;
				while (count-- > 0) {
					byte v = *r++;
					if (v != 0) {
						x++;
						*w++ = v;
					}
					else {
						count--;
						v = *r++;
						if (x + v > cx)
							v = (byte)(cx - x);
						x += v;
						while (v-- != 0)
							*w++ = 0;
					}
				}
			}
			return (uint)(w - dst);
		}
			

	}
}
