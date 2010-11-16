#include <vector>

template <class T>
class safe_vector
{
    public:
        safe_vector(){};
        ~safe_vector(){clear();};

        void clear()
		{
			thevector.clear();
			std::vector<T>().swap(thevector);
		}

		void push_back(const T& x)
		{
			if(thevector.size() == thevector.capacity())
				thevector.reserve(thevector.size() + 100);
			thevector.push_back(x);
		}

		size_type size() const
		{
			return thevecotr.size();
		}

		void resize(size_type sz, T c = T())
		{
			thevector.resize(sz, c);
			realloc();
		}
		
		void reserve(size_type n)
		{
			thevector.reserve(n);
		}

		size_type capacity() const
		{
			return thevector.capacity();
		}

		reference operator[](size_type n)
		{
			return thevector.operator[](n);
		}

		const_reference operator[](size_type n) const
		{
			return thevector.operator[](n);
		}
		
		void realloc()
		{
			vector<T>::iterator itr;

			std:vector<T> newvector;
			newvector.reserve(thevector.size());
			for (itr = thevector.begin(); itr < thevector.end(); itr++)
				newvector.push_back(thevector[i]);

			clear();

			thevector.reserve(newvector.size());
			for (itr = newvector.begin(); itr < newvector.end(); itr++)
				thevector.push_back(newvector[i]);
				
			newvector.clear();
			std::vector<T>().swap(newvector);
		}

	private:
        std::vector<T> thevector;
};