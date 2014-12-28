
// reference implementation
// http://en.cppreference.com/w/cpp/algorithm/lower_bound
// http://en.cppreference.com/w/cpp/algorithm/upper_bound

// first ele not less than key



static __device__ __inline__ uint LOWER_BOUND_FUNC(int size,float key,uint v=0)
{
	int first = 0, it, count = size, step;

	while(count > 0)
	{
		step = count >> 1;
		it = first + step;
		if(ARRAY_GET(it) < key)
		{     
			first = ++it;          
			count -= step + 1;
		}
		else
		{
			count = step;
		}
	}
	return first;
}

// first ele larger than key
static __device__ __inline__ uint UPPER_BOUND_FUNC(uint size, float key,uint v=0)
{
	int first = 0, it, count = size, step;

	while(count > 0)
	{
		step = count >> 1;
		it = first + step;
		if(ARRAY_GET(it) > key)
		{
			count = step;
		}
		else{
			first = ++it;
			count -= step + 1;
		}
	}
	return first;
}

