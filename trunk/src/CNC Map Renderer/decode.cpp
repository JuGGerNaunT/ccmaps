#include "decode.h"


int Decode_64(const unsigned char* r, unsigned char* d) {

	unsigned char* w = d;
	while (*r)
	{
		int c1 = *r++;
		if (decode64_table[c1] == -1)
			return 0;
		int c2 = *r++;
		if (decode64_table[c2] == -1)
			return 0;
		int c3 = *r++;
		if (c3 != '=' && decode64_table[c3] == -1)
			return 0;
		int c4 = *r++;
		if (c4 != '=' && decode64_table[c4] == -1)
			return 0;
		*w++ = (decode64_table[c1] << 2) | (decode64_table[c2] >> 4);
		if (c3 == '=')
			break;
		*w++ = ((decode64_table[c2] << 4) & 0xf0) | (decode64_table[c3] >> 2);
		if (c4 == '=')
			break;
		*w++ = ((decode64_table[c3] << 6) & 0xc0) | decode64_table[c4];
	}
	return w - d;
}


int Read_Word(const unsigned char*& r) {
	int v = *reinterpret_cast<const unsigned short*>(r);
	r += 2;
	return v;
}

int Get_Count(const unsigned char*& r) {
	int count = -255;
	int v;
	do
	{
		count += 255;
		v = *r++;
	}
	while (!v);
	return count + v;
}

// Decode LZO compressed data chunks
int Decode_5_s(const unsigned char* s, unsigned char* d, int cb_s) {
	lzo_init();
	lzo_uint cb_d;
	if (LZO_E_OK != lzo1x_decompress(s, cb_s, d, &cb_d, 0))
		return 0;
	return cb_d;
}

// Decodes LZO data in chunks
int Decode_5(const unsigned char* s, unsigned char* d, int cb_s, int format)
{
	const unsigned char* r = s;
	const unsigned char* r_end = s + cb_s;
	unsigned char* w = d;
	while (r < r_end)
	{
		const t_pack_section_header& header =
			*reinterpret_cast<const t_pack_section_header*>(r);
		r += sizeof(t_pack_section_header);
		if (format == 80)
			Decode_80(r, w);
		else
			Decode_5_s(r, w, header.size_in);
		r += header.size_in;
		w += header.size_out;
	}
	return w - d;
}

// Decodes old SHP images, Overlay sections, maybe more
int Decode_80(const unsigned char image_in[], unsigned char image_out[])
{
    #ifndef WIN32
	int cb_out;
	/*
	0 copy 0cccpppp p
	1 copy 10cccccc
	2 copy 11cccccc p p
	3 fill 11111110 c c v
	4 copy 11111111 c c p p
	*/

	_asm
	{
			push	esi
			push	edi
			mov		ax, ds
			mov		es, ax
			mov		esi, image_in
			mov		edi, image_out
next:
			xor		eax, eax
			lodsb
			mov		ecx, eax
			test	eax, 0x80
			jnz		c1c
			shr		ecx, 4
			add		ecx, 3
			and		eax, 0xf
			shl		eax, 8
			lodsb
			mov		edx, esi
			mov		esi, edi
			sub		esi, eax
			jmp		copy_from_destination
c1c:
			and		ecx, 0x3f
			test	eax, 0x40
			jnz		c2c
			or		ecx, ecx
			jz		end
			jmp		copy_from_source
c2c:
			xor		eax, eax
			lodsw
			cmp		ecx, 0x3e
			je		c3
			ja		c4
			mov		edx, esi
			mov		esi, image_out
			add		esi, eax
			add		ecx, 3
			jmp		copy_from_destination
c3:
			mov		ecx, eax
			lodsb
			rep		stosb
			jmp		next
c4:
			mov		ecx, eax
			lodsw
			mov		edx, esi
			mov		esi, image_out
			add		esi, eax
copy_from_destination:
			rep		movsb
			mov		esi, edx
			jmp		next
copy_from_source:
			rep		movsb
			jmp		next
end:
			sub		edi, image_out
			mov		cb_out, edi
			pop		edi
			pop		esi
	}
	return cb_out;
	#else

    const unsigned char* copyp;
    const unsigned char* readp = image_in;
    unsigned char* writep = image_out;
    unsigned int code;
    unsigned int count;

    while (1) {
        code = *readp++;
        if (~code & 0x80) {
            //bit 7 = 0
            //command 0 (0cccpppp p): copy
            count = (code >> 4) + 3;
            copyp = writep - (((code & 0xf) << 8) + *readp++);
            while (count--)
                *writep++ = *copyp++;
        } else {
            //bit 7 = 1
            count = code & 0x3f;
            if (~code & 0x40) {
                //bit 6 = 0
                if (!count)
                    //end of image
                    break;
                //command 1 (10cccccc): copy
                while (count--)
                    *writep++ = *readp++;
            } else {
                //bit 6 = 1
                if (count < 0x3e) {
                    //command 2 (11cccccc p p): copy
                    count += 3;
                    copyp = &image_out[*(unsigned short*)readp];

                    readp += 2;
                    while (count--)
                        *writep++ = *copyp++;
                } else if (count == 0x3e) {
                    //command 3 (11111110 c c v): fill
                    count = *(unsigned short*)readp;
                    readp += 2;
                    code = *readp++;
                    while (count--)
                        *writep++ = code;
                } else {
                    //command 4 (copy 11111111 c c p p): copy
                    count = *(unsigned short*)readp;
                    readp += 2;
                    copyp = &image_out[*(unsigned short*)readp];
                    readp += 2;
                    while (count--)
                        *writep++ = *copyp++;
                }
            }
        }
    }

    return (writep - image_out);

	#endif
}

