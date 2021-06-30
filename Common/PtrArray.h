///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#ifndef PTRARRAY_H
#define PTRARRAY_H

template <typename T>
class CPtrArray
{
public:
	typedef T* Type;

	CPtrArray(int allocate = 0)
		: m_size(0),
		m_dataSize(allocate),
		m_data(null)
	{
		if (m_dataSize > 0)
			m_data = new Type[m_dataSize];
	}

	~CPtrArray()
	{
		DeleteArray(m_data);
	}

	int GetSize() const
	{
		return m_size;
	}

	void Allocate(int size)
	{
		if (size > m_dataSize)
		{
			m_dataSize = size;
			Type* newData = new Type[m_dataSize];
			if (m_size > 0)
				memcpy(newData, m_data, sizeof(Type) * m_size);
			DeleteArray(m_data);
			m_data = newData;
		}
	}

	void Append(const Type& ptr, int pos = -1)
	{
		if (m_size + 1 > m_dataSize)
		{
			m_dataSize = m_size + 1;
			Type* newData = new Type[m_dataSize];
			if (m_size > 0)
				memcpy(newData, m_data, sizeof(Type) * m_size);
			DeleteArray(m_data);
			m_data = newData;
		}

		if (pos == -1
			|| pos >= m_size
			|| m_size == 0)
		{
			m_data[m_size] = ptr;
		}
		else
		{
			memcpy(m_data + pos + 1, m_data + pos, sizeof(Type) * (m_size - pos));
			m_data[pos] = ptr;
		}

		m_size++;
	}

	void RemoveAt(int pos)
	{
		if (pos < 0 || pos >= m_size)
			return;

		if (pos < m_size - 1)
			memcpy(m_data + pos, m_data + pos + 1, sizeof(Type) * (m_size - pos - 1));

		m_size--;
	}

	void RemoveAll()
	{
		m_size = 0;
	}

	Type& operator[](int i)
	{
		return m_data[i];
	}

	const Type& operator[](int i) const
	{
		return m_data[i];
	}

	Type& GetAt(int i)
	{
		return m_data[i];
	}

	const Type& GetAt(int i) const
	{
		return m_data[i];
	}

	int Find(const Type& ptr) const
	{
		for (int i = 0; i < m_size; i++)
			if (m_data[i] == ptr)
				return i;
		return -1;
	}

	bool Contains(const Type& ptr) const
	{
		for (int i = 0; i < m_size; i++)
			if (m_data[i] == ptr)
				return true;
		return false;
	}

	void Sort(int(*compar)(const Type*, const Type*))
	{
		if (m_size > 1)
			qsort((void*)m_data, m_size, sizeof(Type), (int(*)(const void*, const void*))compar);
	}

private:
	int m_size;
	int m_dataSize;
	Type* m_data;

	CPtrArray(const CPtrArray&);
	CPtrArray& operator=(const CPtrArray&);
};

#endif // PTRARRAY_H