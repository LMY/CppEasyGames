#include <unordered_map>
#include <iostream>
#include <vector>
#include <memory>

class Resource {};
class AudioResource : Resource {};
class ImageResource : Resource {};
class VideoResource : Resource {};
class MeshResource : Resource {};
class StringResource : Resource {
	std::string value; 
public:
	StringResource(std::string s) : value(s) {}
	
	std::string get() const { return value; }
	void set(std::string s)	{ value = s; }
};

template<template<typename ...> class SmartPointer = std::unique_ptr, template <typename ...> class Container = std::unordered_map, typename KeyType = unsigned int>
class ResourceManager
{
	Container<KeyType, SmartPointer<StringResource> > strings;
	Container<KeyType, SmartPointer<ImageResource> > images;
	Container<KeyType, SmartPointer<AudioResource> > audio;
	Container<KeyType, SmartPointer<VideoResource> > videos;
	Container<KeyType, SmartPointer<MeshResource> > meshes;
	
	//Container<KeyType, StringResource> resources[5];
	KeyType maxID[5];
	
public:
	enum class ResourceType : unsigned char { STRING=0, IMAGE=1, AUDIO=2, VIDEO=3, MESH=4 };
	KeyType NextKey(ResourceType type) { return maxID[static_cast<int>(type)]+1; }
	/*
	Resource& get(ResourceType type, KeyType key) {
		switch (type) {
			case ResourceType::STRING : return *strings.get(key);
			case ResourceType::IMAGE : return *images.get(key);
			case ResourceType::AUDIO : return *audio.get(key);
			case ResourceType::VIDEO : return *videos.get(key);
			case ResourceType::MESH : return *meshes.get(key);
		}
	}*/
	/*
	void put(ResourceType type, const Resource& res, KeyType key) {
		switch (type) {
			case ResourceType::IMAGE : images[key] = SmartPointer<ImageResource>(std::move(res)); break;
			case ResourceType::AUDIO : audio[key] = SmartPointer<AudioResource>(std::move(res)); break;
			case ResourceType::VIDEO : videos[key] = SmartPointer<VideoResource>(std::move(res)); break;
			case ResourceType::MESH : meshes[key] = SmartPointer<MeshResource>(std::move(res)); break;
			case ResourceType::STRING : strings[key] = SmartPointer<StringResource>(std::move(res)); break;
		}
		
		const int ix = static_cast<int>(type);
		if (key > maxID[ix]) maxID[ix] = key;
	}*/
	
	KeyType insert(ResourceType type, const Resource& res) {
		const KeyType newkey = NextKey(type);
		put(type, res, newkey);
		return newkey;
	}
	
	ImageResource& getImage(KeyType i) const { return *images[i]; }
	AudioResource& getAudio(KeyType i) const { return *audio[i]; }
	VideoResource& getVideo(KeyType i) const { return *videos[i]; }
	MeshResource& getMesh(KeyType i) const { return *meshes[i]; }
	StringResource& getString(KeyType i) const { return *strings[i]; }
	/*
	void putImage(KeyType key, const ImageResource& res) { if (key > maxID[1]) maxID[1] = key; images[key] = SmartPointer<ImageResource>(res); }
	void putAudio(KeyType key, const AudioResource& res) { if (key > maxID[2]) maxID[2] = key; audio[key] = SmartPointer<AudioResource>(res); }
	void putVideo(KeyType key, const VideoResource& res) { if (key > maxID[3]) maxID[3] = key; videos[key] = SmartPointer<VideoResource>(res); }
	void putMesh(KeyType key, const MeshResource& res) { if (key > maxID[4]) maxID[4] = key; meshes[key] = SmartPointer<MeshResource>(res); }*/
	void putString(KeyType key, const StringResource& res) { if (key > maxID[0]) maxID[0] = key; strings[key] = SmartPointer<StringResource>(std::move(res)); }
	
	size_t imagesLen() const { return images.size(); }
	size_t audiosLen() const { return audio.size(); }
	size_t videosLen() const { return videos.size(); }
	size_t meshesLen() const { return meshes.size(); }
	size_t stringsLen() const { return strings.size(); }
};
typedef ResourceManager<> SimpleResourceManager;



int main()
{
	SimpleResourceManager manager;
	manager.putString(0, StringResource(std::string("hello")));
	manager.putString(1, StringResource(std::string("world")));
	
	std::cout << manager.imagesLen() << manager.getString(0).get()  <<  std::endl;
	
	return 0;
}
