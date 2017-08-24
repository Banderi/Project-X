#include <regex>		// required for std::find_if			(Antimony::spawn)

#include "Spawner.h"
#include "Bullet.h"
#include "Console.h"

///

struct find_id
{
	std::wstring id;
	find_id(std::wstring id) : id(id) {}
	bool operator () (const SpawnItem& m) const
	{
		return m.id == id;
	}
};

///

bool Antimony::addSpawnable(std::wstring id, antSpawnCallback call)
{
	SpawnItem item;
	item.id = id;
	item.spawn = call;
	spawnables.push_back(item);

	return true;
}
bool Antimony::spawn(std::wstring id, unsigned int qu, float3 pos)
{
	auto item = std::find_if(spawnables.begin(), spawnables.end(), find_id(id));

	if (item == spawnables.end())
	{
		devConsole.log(L"Unrecognized object id: '" + id + L"'\n", CSL_ERR_GENERIC);
		return false;
	}

	bool success = false;

	for (UINT i = 0; i < qu; i++)
	{
		success = item->spawn(id, pos);
	}
	return success;
}
bool Antimony::standardSpawn(std::wstring id, float3 pos)
{
	if (id == L"cube")
	{
		btDefaultMotionState *ms = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), WORLD_SCALE * Float3Tobt(&pos)));
		btObject *phys_obj = new btObject(BTOBJECT_DYNAMIC, BTSOLID_BOX, 10.0f, float3(0.3, 0.3, 0.3), ms, Antimony::btWorld);
		Antimony::addPhysEntity(phys_obj);
		return true;
	}
	else if (id == L"test")
	{
		Antimony::devConsole.log(L"callback test\n", CSL_SUCCESS);
		return true;
	}
	return false;
}

namespace Antimony
{
	std::vector<SpawnItem> spawnables;
}