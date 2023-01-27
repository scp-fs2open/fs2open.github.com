
#include "globalincs/pool.h"
namespace scp_pool{
	int next_uid = 0;
	int new_uid(){ 
		auto i = next_uid;
		next_uid++;
		return i; 
	}
}