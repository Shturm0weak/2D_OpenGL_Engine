#include "MeshManager.h"
#include "../FbxLoader/fbxdocument.h"
#include "../Core/ThreadPool.h"
#include "../Objects/GameObject.h"
#include "../STLloader/STLloader.h"
#include "Instancing.h"
#include "ObjLoader/OBJLoader.h"

using namespace Doom;

MeshManager& Doom::MeshManager::GetInstance()
{
	static MeshManager instance;
	return instance;
}

int Doom::MeshManager::GetAmountOfMeshes()
{
	return s_Meshes.size();
}

void MeshManager::LoadMesh(std::string name, std::string filePath, size_t meshId)
{
	if (meshId > 0)
		name = name.append(std::to_string(meshId));
	auto iter = s_Meshes.find(name);
	if (iter != s_Meshes.end())
		return;
	std::string subStr = filePath.substr(filePath.size() - 3, 3);
	if (subStr == "fbx")
	{
		fbx::FBXDocument doc;
		s_Meshes.insert(std::make_pair(name, doc.LoadMesh(name, filePath, meshId)));
	}
	else if (subStr == "stl") 
		s_Meshes.insert(std::make_pair(name, StlLoader::LoadMesh(name, filePath)));
	else if (subStr == "obj")
	{
		Mesh* mesh = objl::Loader::Load(filePath);
		s_Meshes.insert(std::make_pair(mesh->m_Name, mesh));
	}
	else return;
	std::vector<Renderer3D*> New;
#define	_LOAD_MESH_
	Mesh* mesh = GetMesh(name);
#undef _LOAD_MESH_
	if (mesh == nullptr) return;
	mesh->m_IdOfMeshInFile = meshId;
	mesh->Init();
	Instancing::GetInstance()->m_InstancedObjects.insert(std::make_pair(mesh, New));
	Instancing::GetInstance()->Create(mesh);
	Logger::Success("has been loaded!", "Mesh", name.c_str());
}

void Doom::MeshManager::LoadScene(std::string filepath)
{
	fbx::FBXDocument doc;
	doc.LoadScene(filepath);
}

void Doom::MeshManager::AsyncLoadMesh(std::string nametemp, std::string filePath, size_t meshId)
{
	Doom::ThreadPool::GetInstance().Enqueue([=] {
		std::lock_guard<std::mutex> lock(s_Mtx);
		std::string name = nametemp;
		if (meshId > 0)
			name = name.append(std::to_string(meshId));
		
		auto iter = s_Meshes.find(name);
		if (iter != s_Meshes.end()) return;
		std::string subStr = filePath.substr(filePath.size() - 3, 3);
		Mesh* mesh = nullptr;
		if (subStr == "fbx") 
		{
			fbx::FBXDocument doc;
			mesh = doc.LoadMesh(name, filePath, meshId);
		}
		else if (subStr == "stl")
			mesh = StlLoader::LoadMesh(name, filePath);
		else if (subStr == "obj")
		{
			mesh = objl::Loader::Load(filePath);
			s_Meshes.insert(std::make_pair(mesh->m_Name, mesh));
		}
		else return;
		if (mesh == nullptr) return;
		{
			
			s_Meshes.insert(std::make_pair(name, mesh));
			s_NeedToInitMeshes.push_back(mesh);
		}
		Logger::Success("has been loaded!", "Mesh", name.c_str());
		});
}

Mesh* MeshManager::GetMesh(std::string name)
{
	auto iter = s_Meshes.find(name);
	if (iter != s_Meshes.end()) 
		return iter->second;
	else
	{
#ifdef _LOAD_MESH_
		Logger::Warning("has not been found!", "Mesh", name.c_str());
#endif
		return nullptr;
	}
}

const char** Doom::MeshManager::GetListOfMeshes()
{
	if (s_NamesOfMeshes != nullptr)
		delete[] s_NamesOfMeshes;
	s_NamesOfMeshes = new const char* [s_Meshes.size()];
	size_t i = 0;
	for (auto mesh = s_Meshes.begin(); mesh != s_Meshes.end(); mesh++)
	{
		s_NamesOfMeshes[i] = mesh->first.c_str();
		i++;
	}
	return s_NamesOfMeshes;
}

void Doom::MeshManager::AddMesh(Mesh* mesh)
{
	if (mesh != nullptr)
	{
		s_Meshes.insert(std::make_pair(mesh->m_Name, mesh));
	}
}

void MeshManager::GetMeshWhenLoaded(std::string name, void* r)
{
	//auto iter = Meshes.find(name);
	//if (iter == Meshes.end()) {
	s_MeshQueue.insert(std::make_pair(name, r));
	//}
	//else {
	//	Renderer3D* render = static_cast<Renderer3D*>(r);
	//	render->mesh = GetMesh(name);
	//}
}

void MeshManager::DeleteMesh(std::string name)
{
	auto iter = s_Meshes.find(name);
	if (iter != s_Meshes.end())
	{
		Mesh* mesh = iter->second;
		s_Meshes.erase(iter);
		delete mesh;
	}
}

void MeshManager::DeleteMesh(Mesh* mesh)
{
	DeleteMesh(mesh->m_Name);
}

void MeshManager::ShutDown()
{
	for (auto i = s_Meshes.begin(); i != s_Meshes.end(); i++)
	{
		Mesh* mesh = i->second;
		delete mesh;
	}
	s_Meshes.clear();
}

void MeshManager::DispatchLoadedMeshes()
{
	for (size_t i = 0; i < s_NeedToInitMeshes.size();)
	{
		s_NeedToInitMeshes.back()->Init();
		std::vector<Renderer3D*> New;
		Instancing::GetInstance()->m_InstancedObjects.insert(std::make_pair(s_NeedToInitMeshes[i], New));
		Instancing::GetInstance()->Create(s_NeedToInitMeshes[i]);
		s_NeedToInitMeshes.pop_back();
	}
	if (s_MeshQueue.size() > 0)
	{
		for (auto i = s_MeshQueue.begin(); i != s_MeshQueue.end();)
		{
			auto iter = s_Meshes.find(i->first);
			if (iter != s_Meshes.end())
			{
				Renderer3D* r = static_cast<Renderer3D*>(i->second);
				r->LoadMesh(iter->second);
				r->ChangeRenderTechnic(r->m_RenderTechnic);
				s_MeshQueue.erase(i++);
			}
			else
				i++;
		}
	}
}