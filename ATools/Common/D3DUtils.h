///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#ifndef D3DUTILS_H
#define D3DUTILS_H

namespace D3D
{
	TCHAR* D3DUtil_D3DFormatToString(D3DFORMAT format, bool bWithPrefix);

	// Vertex Proccessing types enum, including the (hardware) pure device variant
	enum VPTYPE
	{
		SOFT_VP,
		MIXD_VP,
		HARD_VP,
		PURE_VP,
		FORCE_DWORD_VP = 0x7fffffff
	};

	// bits in a D3D format inspectors
	UINT RGBBITS(D3DFORMAT fmt);
	UINT ALPHABITS(D3DFORMAT fmt);
	UINT DEPTHBITS(D3DFORMAT fmt);
	UINT STENCILBITS(D3DFORMAT fmt);


	// D3D type to string converters, with optional prefix skipping
	TCHAR* VPTYPESTRING(VPTYPE vpt);
	TCHAR* DEVICETYPESTRING(D3DDEVTYPE dt, bool bPrefix = true);
	TCHAR* MULTISAMPLESTRING(D3DMULTISAMPLE_TYPE mst, bool bPrefix = true);
	TCHAR* PRESENTINTERVALSTRING(UINT pi, bool bPrefix = true);

	//----------------------------------------------------------------------------
	// CTArray.h: C++ template based class for dynamic arrays.
	//
	// 10/15/05 - Michael B. Comet, comet@comet-cartoons.com - Initial Rev
	// 05/20/06 - tor - no ostream cout friends, renamed most of the members.
	// 06/05/06 - tor - qsort wrapper with input sort callback
	// 06/21/06 - tor - Find returning an index, not a boolean
	// 08/23/06 - tor - Find with optional start index, to find duplicates
	// 08/24/06 - tor - GetAll to a void ptr of specified size
	// 08/25/06 - tor - some methods are now virtual for subclass overrides
	// 08/26/06 - tor - swap
	//----------------------------------------------------------------------------
	template<class TYPE> class CTArray
	{
	private:
		UINT	nSize;	// actual size
		UINT	nGrow;	// grow factor

	protected:
		UINT	nItems;	// number of elements (as it appears to the user)
		TYPE*	pData;	// pointer to array of data

	public:
		// blank constructor
		CTArray() { Init(); }

		// copy constructor
		CTArray(CTArray& Src) { Init(); Copy(Src); }

		// typed copy constructor
		CTArray(const TYPE* pSrc, UINT nCount = 1)
		{
			Init();
			SetLength(nCount);

			for (UINT u = 0; u < nItems; u++) pData[u] = pSrc[u];
		}

		// typed initializing constructor
		CTArray(UINT nCount, TYPE Src)
		{
			Init();
			SetLength(nCount);

			for (UINT u = 0; u < nItems; u++) pData[u] = Src;
		}

		// operators
		const TYPE& operator[](UINT nIndex) const { return pData[nIndex]; }
		TYPE&		operator[](UINT nIndex)		  { return pData[nIndex]; }
		CTArray&	operator=(CTArray& Src)		  { Copy(Src); return *this; }

		// initialise
		void Init() { pData = NULL; nSize = nItems = 0; nGrow = 8; }

		// release data
		virtual void Clear(void) { if (pData != NULL) delete[] pData; Init(); }

		// copy from other
		void Copy(CTArray& Src)
		{
			Clear();
			SetLength(Src.Length());

			for (UINT u = 0; u < nItems; u++) pData[u] = Src.pData[u];
		}

		// grow factor get/set
		UINT GrowFactor(void) const { return nGrow; }
		void SetGrowFactor(UINT nNewGrow) { nGrow = nNewGrow; if (nGrow == 0) nGrow = 1; }

		// length (items) get
		UINT Length(void) { return nItems; }

		// set length regrow or shrink
		virtual bool SetLength(UINT nLength, bool bForce = false)
		{
			if (nLength == 0)
			{
				Clear();
				return true;
			}

			// alloc new storage
			TYPE* pNewData = NULL;

			UINT nNewSize = ((nLength / nGrow) + 1) * nGrow;

			// grow only if either the amount we need is greater than what we have
			// already or if the amount is <= 1/2, whatever's smaller
			if (nNewSize > nSize || nNewSize <= nSize / 2 || bForce)
			{
				if ((pNewData = new TYPE[nNewSize]) == NULL)
					return false;

				// now copy the old elements into the new array, up to the old
				// number of items or to the user-set new length,  whichever's
				// smaller
				for (UINT u = 0; u < nItems && u < nLength; u++)
					pNewData[u] = pData[u];

				// update all the current info...
				if (pData != NULL)
					delete[] pData;

				pData = pNewData;
				nSize = nNewSize;
			}

			nItems = nLength;

			return true;
		}

		// set w/bounds check but no grow
		virtual bool Set(UINT nIndex, TYPE Src) const
		{
			if (nIndex >= nItems || pData == NULL)
				return false;

			pData[nIndex] = Src;

			return true;
		}

		// get w/bounds check but no grow
		virtual bool Get(TYPE& Dst, UINT nIndex) const
		{
			if (nIndex >= nItems || pData == NULL)
				return false;

			Dst = pData[nIndex];

			return true;
		}

		// get all elements to a typed pointer; do not forget to delete
		// such pointer after no longer needed
		UINT GetAll(TYPE*& pDst)
		{
			pDst = new TYPE[nItems];

			for (UINT u = 0; u < nItems; u++)
				pDst[u] = pData[u];

			return nItems;
		}

		// get all elements to an unknown size pointer of specified size; the
		// pointer must be initialized by the caller...
		UINT GetAll(void* pDst, int nSize)
		{
			for (UINT u = 0; u < nItems; u++)
				memcpy((void*)((BYTE*)pDst + u * nSize), (void*)(&pData[u]), nSize);

			return nItems;
		}

		// remove element at given position
		virtual bool Remove(UINT nIndex)
		{
			if (nItems == 0 || pData == NULL)
				return false;

			// starting with the element we are removing, work up
			// copying each next value down to the current spot...
			for (UINT u = nIndex; u < nItems - 1; u++)
				pData[u] = pData[u + 1];

			// this will either simply change the nItems value or realloc and
			// free some memory
			SetLength(nItems - 1);

			return true;
		}

		// insert element at given position
		virtual void Insert(TYPE Src, UINT nIndex)
		{
			// first, make room
			SetLength(nItems + 1);

			// starting with the last element work back until we get to the one
			// we are inserting at and copy forward
			for (UINT u = nItems - 1; u > nIndex; u--)
				pData[u] = pData[u - 1];

			// finally insert new value
			pData[nIndex] = Src;
		}

		// append element to the end of array
		virtual int Append(TYPE Src)
		{
			// first, make room
			SetLength(nItems + 1);

			// insert new value
			pData[nItems - 1] = Src;

			return nItems;
		}

		// blank append
		virtual int Append()
		{
			// just make room
			SetLength(nItems + 1);

			return nItems;
		}

		// finder with mem compare and optional start
		int Find(TYPE Src, UINT nStart = 0)
		{
			for (UINT u = nStart; u < nItems; u++)
			{
				if (memcmp(&pData[u], &Src, sizeof(TYPE)) == 0)
					return (int)u;
			}

			return -1;
		}

		// swap
		void Swap(UINT i, UINT j)
		{
			if (i >= nItems || j >= nItems || i == j)
				return;

			TYPE Tmp = pData[i];
			pData[i] = pData[j];
			pData[j] = Tmp;
		}

		// sort wrapper with callback and method
		void Sort(int(__cdecl* compare)(const void* p1, const void* p2), int nMethod = 0)
		{
			switch (nMethod)
			{
			case 1:
			{
				// sort the array with fixed starting items, by comparing neighbors
				for (UINT i = 0; i < nItems - 1; i++)
				{
					// skip a neighbor that is in order (as determined by a non-zero
					// return from the compare function)
					if (compare((const void*)&pData[i], (const void*)&pData[i + 1]))
						continue;

					// search for an item matching the last starting item
					UINT j = i + 1;

					while (!compare((const void*)&pData[i], (const void*)&pData[j]) && j < nItems - 1)
						j++;

					// swap the matching item to be right below the starting item
					if (j <= nItems - 1)
						Swap(i + 1, j);
				}

				break;
			}
			default:
				qsort(pData, nItems, sizeof(TYPE), compare);
				break;
			}
		}

		// destructor
		~CTArray() { Clear(); }
	};

	typedef CTArray<DWORD> DWORDARRAY;
}

#endif // D3DUTILS_H