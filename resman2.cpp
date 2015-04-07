#include <unordered_map>
#include <iostream>
#include <vector>
#include <memory>


template<typename ResourceType,
	template<typename ...> class SmartPointer = std::unique_ptr,
	template <typename ...> class Container = std::unordered_map,
	typename KeyType = unsigned int>
class ResourceCollection
{
	Container<KeyType, SmartPointer<ResourceType> > resources;
	KeyType maxID;
	
public:
	ResourceCollection() : resources(), maxID(0) {}

	void insert(KeyType key, ResourceType* res) {
		resources[key]=SmartPointer<ResourceType>(res);
		if (key > maxID)
			maxID = key;
		std::cout << "id: " << maxID << std::endl;
	}
	void push_back(ResourceType* res) { insert(maxID+1, res); }
	
	ResourceType& get(KeyType key) { return *resources[key]; }
	
	size_t size() const { return resources.size(); }
};

typedef ResourceCollection<std::string> StringCollection;



int main()
{
	StringCollection s;
	s.insert(0, new std::string("hello"));
	s.insert(1, new std::string("world"));
	//s.push_back(new std::string("hello"));
	//s.push_back(new std::string("world"));
		
	std::cout << s.size() << " " << s.get(0)  << " " << s.get(1) <<  std::endl;
	
	return 0;
}
