#include "pch.h"
#include "ShipEnemy.h"
#include "Bullet.h"

void ShipEnemy::OnCollision(void * _col)
{
	Collision* __col = static_cast<Collision*>(_col);
	if (__col->GetTag() == "Bullet") {
		Bullet* bullet = static_cast<Bullet*>(__col->GetOwner());
		hp -= bullet->damage;
		bullet->Death();
	}
}

ShipEnemy::ShipEnemy(std::string name, float x, float y) : GameObject(name, x, y)
{
	SoundManager::CreateSoundAsset("explosion",explosionSound);
	bulletsPlaceHolder = new GameObject("bulletsPlaceHolder", 0, 0);
	bulletsPlaceHolder->Enable = false;
	AddChild((void*)bulletsPlaceHolder);
	bulletsPlaceHolder->SetOwner((void*)this);
	EventSystem::Instance()->RegisterClient("OnUpdate", (GameObject*)this);
	EventSystem::Instance()->RegisterClient("OnStart", (GameObject*)this);
	EventSystem::Instance()->RegisterClient("OnCollision", (GameObject*)this);
	col = GetComponentManager()->AddComponent<Collision>();
	tr = GetComponentManager()->GetComponent<Transform>();
	tr->Scale(5, 5);
	tr->RotateOnce(180, glm::vec3(0, 0, 1));
	SetColor(COLORS::Red);
	SetTexture(texture);
	col->IsTrigger = false;
	for (unsigned int i = 0; i < amountOfBulletsInPool; i++)
	{
		bullets.push_back(new Bullet("EnemyBullet", glm::vec3(0,-1,0), "Bullet" + std::to_string(i), position.x, position.y));
		bullets[i]->isActive = false;
		bullets[i]->SetOwner((void*)bulletsPlaceHolder);
		bulletsPlaceHolder->AddChild((void*)bullets[i]);
		bullets[i]->Enable = false;
		bullets[i]->col->Enable = false;
		bullets[i]->damage = 10;
	}
}

void ShipEnemy::OnUpdate() {
	if (Enable == false)
		return;

	if (hp <= 0) {
		Death();
	}

	//ThreadPool::Instance()->enqueue([=] {
	col->IsCollidedSAT();// });
	Fire();

	if (position.y < -30)
		Death();

	if (isDead == false)
		tr->Move(0, -3, 0);
}

void ShipEnemy::Spawn()
{
	hp = 100;
	Enable = true;
	col->Enable = true;
	isDead = false;
	tr->Translate(position.x, 20);
}

void ShipEnemy::Death()
{
	pl->kills++;
	Enable = false;
	col->Enable = false;
	isDead = true;
	SoundManager::Play(explosionSound);
}

void ShipEnemy::Fire() {
	if (timerFire > TimePerBullet) {
		timerFire = 0;
		if (usedBulletCounter == amountOfBulletsInPool)
			usedBulletCounter = 0;
		bullets[usedBulletCounter]->SetMoveDirection(glm::vec3(0,-1,0));
		bullets[usedBulletCounter]->tr->Translate(position.x, position.y);
		bullets[usedBulletCounter]->Enable = true;
		bullets[usedBulletCounter]->col->Enable = true;
		bullets[usedBulletCounter]->isActive = true;
		bullets[usedBulletCounter]->lifeTimer = 0;
		usedBulletCounter++;
	}
	timerFire += DeltaTime::deltatime;
}

ShipEnemy::~ShipEnemy()
{
	delete texture;
	delete col;
	delete tr;
}