#define PLAY_IMPLEMENTATION
#define PLAY_USING_GAMEOBJECT_MANAGER
#include "Play.h"

int DISPLAY_WIDTH = 1280;
int DISPLAY_HEIGHT = 720;
int DISPLAY_SCALE = 1;

enum Agent8State
{
	STATE_LEVEL_START = 0,
	STATE_LEVEL,
	STATE_ATTACHED,
	STATE_FLYING,
	STATE_DEAD,
	STATE_LEVEL_COMPLETE
};

struct GameState
{
	
	int level = 1; 
	int gem_remaining = level + 1;

	Agent8State agentState = STATE_LEVEL_START;
};
GameState gameState;

enum GameObjectType
{
	TYPE_NULL = -1,
	TYPE_AGENT8,
	TYPE_ASTEROID,
	TYPE_ASTEROID_ATTACHED,
	TYPE_ASTEROID_PIECES,
	TYPE_METEOR,
	TYPE_GEM,
	TYPE_DESTROYED,
	TYPE_PARTICLE
};

void CreateAsteroids();
void CreateMeteors();

void UpdatePlayerAttached();
void UpdatePlayerFlying();
void UpdateParticleTrails();
void UpdateAsteroid();
void UpdateAsteroidAttached();
void UpdateMeteor();
void UpdateGems();
void UpdateDestroyed();
void UpdateAgent8();
void IfObjectOffScreen(GameObject& object, float& x, float& y);

void MainGameEntry( PLAY_IGNORE_COMMAND_LINE )
{
	Play::CreateManager( DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_SCALE );
	Play::CentreAllSpriteOrigins();
	Play::LoadBackground("Data\\Backgrounds\\spr_background.png");
	Play::StartAudioLoop("music");
	Play::MoveSpriteOrigin("asteroid_2", 0, -17);
	Play::MoveMatchingSpriteOrigins("agent8", 0, 62);
	Play::CreateGameObject(TYPE_AGENT8, { DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2}, 30, "agent8");
}

bool MainGameUpdate( float elapsedTime )
{ 
	Play::DrawBackground();
	UpdateParticleTrails();
	UpdateAsteroid();
	UpdateAsteroidAttached();
	UpdateMeteor();
	UpdateGems();
	UpdateAgent8();
	UpdateDestroyed();
	
	Play::DrawFontText("105px", "Gems Remaining: " + std::to_string(gameState.gem_remaining), { DISPLAY_WIDTH / 2, 50 }, Play::CENTRE);
	Play::PresentDrawingBuffer();
	return Play::KeyDown(VK_ESCAPE);
}

int MainGameExit( void )
{
	Play::DestroyManager();
	return PLAY_OK;
}

void CreateAsteroids()
{
	for (int i = 0; i < gameState.level; i++)
	{
		int id_asteroid = Play::CreateGameObject(TYPE_ASTEROID, Point2f(Play::RandomRollRange(0, DISPLAY_WIDTH), Play::RandomRollRange(0, DISPLAY_HEIGHT)), 70, "asteroid_2");
	}

	std::vector<int> vAsteroids = Play::CollectGameObjectIDsByType(TYPE_ASTEROID);
	for (int id : vAsteroids )
	{
		GameObject& obj_asteroid = Play::GetGameObject(id);
		float direction = Play::RandomRollRange(1, 360) * (PLAY_PI / 180);
		float asteroid_speed = 4;
		obj_asteroid.velocity = { sin(direction) * asteroid_speed, cos(direction) * asteroid_speed };
		obj_asteroid.animSpeed = 0.05f;
		obj_asteroid.rotation = PLAY_PI - direction;
	}

	int id_asteroid_attached = Play::CreateGameObject(TYPE_ASTEROID_ATTACHED, Point2f(Play::RandomRollRange(0, DISPLAY_WIDTH), Play::RandomRollRange(0, DISPLAY_HEIGHT)), 70, "asteroid_2");

	GameObject& obj_asteroid_attached = Play::GetGameObject(id_asteroid_attached);
	float direction = Play::RandomRollRange(1, 360) * (PLAY_PI / 180);
	float asteroid_speed = 4;
	obj_asteroid_attached.velocity = { sin(direction) * asteroid_speed, cos(direction) * asteroid_speed };
	obj_asteroid_attached.animSpeed = 0.05f;
	obj_asteroid_attached.rotation = PLAY_PI - direction;
}

void CreateMeteors()
{
	for (int i = 0; i < gameState.level; i++)
	{
		int id_meteor = Play::CreateGameObject(TYPE_METEOR, Point2f(Play::RandomRollRange(0, DISPLAY_WIDTH), Play::RandomRollRange(0, DISPLAY_HEIGHT)), 70, "meteor");
	}

	std::vector<int> vMeteors = Play::CollectGameObjectIDsByType(TYPE_METEOR);
	for (int id : vMeteors)
	{
		GameObject& obj_meteor = Play::GetGameObject(id);
		float direction = Play::RandomRollRange(1, 360) * (PLAY_PI / 180);
		float meteor_speed = 6;
		obj_meteor.velocity = { sin(direction) * meteor_speed, cos(direction) * meteor_speed };
		obj_meteor.animSpeed = 0.05f;
		obj_meteor.rotation = PLAY_PI - direction;
	}
}

void UpdatePlayerAttached()
{
	GameObject& obj_agent8 = Play::GetGameObjectByType(TYPE_AGENT8);
	GameObject& obj_attached = Play::GetGameObjectByType(TYPE_ASTEROID_ATTACHED);
	float angle = obj_agent8.rotation;
	if (Play::KeyDown(VK_LEFT))
	{
		obj_agent8.rotSpeed = -0.02;
		Play::SetSprite(obj_agent8, "agent8_left", 0.2f);

	}
	else if (Play::KeyDown(VK_RIGHT))
	{
		obj_agent8.rotSpeed = 0.02;
		Play::SetSprite(obj_agent8, "agent8_right", 0.2f);
	}
	else
	{
		Play::SetSprite(obj_agent8, "agent8_right", 0);
		obj_agent8.rotSpeed = 0;
	}

	Play::UpdateGameObject(obj_agent8);

	if (Play::IsLeavingDisplayArea(obj_agent8))
	{
		if (!Play::IsVisible(obj_agent8))
		{
			IfObjectOffScreen(obj_agent8, obj_agent8.pos.x, obj_agent8.pos.y);
		}
	}
	
	Play::DrawObjectRotated(obj_agent8);
}

void UpdatePlayerFlying()
{
	GameObject& obj_agent8 = Play::GetGameObjectByType(TYPE_AGENT8);
	GameObject& obj_asteroid = Play::GetGameObjectByType(TYPE_ASTEROID);

	Play::SetSprite(obj_agent8, "agent8_fly", 1.0f);
	float angle = obj_agent8.rotation; 
	float player_speed = 4.0;
	obj_agent8.velocity = { -sin(-angle) * player_speed , -cos(-angle) * player_speed }; 
	
	if (Play::KeyDown(VK_LEFT))
	{
		obj_agent8.rotSpeed = -0.005;
	}
	else if (Play::KeyDown(VK_RIGHT))
	{
		obj_agent8.rotSpeed = 0.005;
	}
	else
	{
		obj_agent8.rotSpeed = 0;
	}

	//Create particles
	int id = Play::CreateGameObject(TYPE_PARTICLE, obj_agent8.pos + Point2f{ Play::RandomRoll(20) - 10,Play::RandomRoll(20) - 10 }, 0, "particle");
	GameObject& obj_particle = Play::GetGameObject(id);
	float spread = Play::RandomRoll(50) / 100.0 + 0.5f;
	obj_particle.scale = spread;

	Play::UpdateGameObject(obj_agent8);

	if (Play::IsLeavingDisplayArea(obj_agent8))
	{
		if (!Play::IsVisible(obj_agent8))
		{
			IfObjectOffScreen(obj_agent8, obj_agent8.pos.x, obj_agent8.pos.y);
		}
	}
	Play::DrawObjectRotated(obj_agent8);
}

void UpdateParticleTrails()
{
	GameObject& obj_agent8 = Play::GetGameObjectByType(TYPE_AGENT8);
	std::vector<int> vParticles = Play::CollectGameObjectIDsByType(TYPE_PARTICLE);

	for (int id_particle : vParticles)
	{
		GameObject& obj_particle = Play::GetGameObject(id_particle);
		obj_particle.animSpeed = 0.1f;
		obj_particle.scale -= 0.01f; 
		Play::UpdateGameObject(obj_particle);
		Play::DrawObjectRotated(obj_particle, (8-obj_particle.frame)/8.0f);
		
		if (!Play::IsVisible(obj_particle) || obj_particle.frame >= 8)
		{
			Play::DestroyGameObject(id_particle);
		}
	}
}

void UpdateAsteroid()
{
	GameObject& obj_agent8 = Play::GetGameObjectByType(TYPE_AGENT8);
	std::vector<int> vAsteroids = Play::CollectGameObjectIDsByType(TYPE_ASTEROID);

	for (int id : vAsteroids)
	{
		GameObject& obj_asteroid = Play::GetGameObject(id);

		if (gameState.agentState == STATE_FLYING && Play::IsColliding(obj_asteroid, obj_agent8))
		{
			obj_agent8.rotation = obj_agent8.oldRot - PLAY_PI;
			gameState.agentState = STATE_ATTACHED;
			obj_asteroid.type = TYPE_ASTEROID_ATTACHED;

		}
		Play::UpdateGameObject(obj_asteroid);
		if (Play::IsLeavingDisplayArea(obj_asteroid))
		{
			if (!Play::IsVisible(obj_asteroid))
			{
				IfObjectOffScreen(obj_asteroid, obj_asteroid.pos.x, obj_asteroid.pos.y);
			}
		}

		Play::DrawObjectRotated(obj_asteroid);
	}
}

void UpdateAsteroidAttached()
{
	GameObject& obj_agent8 = Play::GetGameObjectByType(TYPE_AGENT8);
	std::vector<int> vAttached = Play::CollectGameObjectIDsByType(TYPE_ASTEROID_ATTACHED);
	for (int id_attached : vAttached)
	{
		GameObject& obj_attached = Play::GetGameObject(id_attached);
		obj_agent8.pos = obj_attached.pos;
		Play::UpdateGameObject(obj_attached);

		bool hasCollided = true;
		if (Play::KeyPressed(VK_SPACE))
		{
			Play::PlayAudio("explode");
			obj_attached.type = TYPE_DESTROYED;
			gameState.agentState = STATE_FLYING;

			int id_gem = Play::CreateGameObject(TYPE_GEM, obj_attached.pos, 20, "gem");

		}

		if (Play::IsLeavingDisplayArea(obj_attached))
		{
			if (!Play::IsVisible(obj_attached))
			{
				IfObjectOffScreen(obj_attached, obj_attached.pos.x, obj_attached.pos.y);
			}
		}

		Play::DrawObjectRotated(obj_attached);
	}
}

void UpdateMeteor()
{
	GameObject& obj_agent8 = Play::GetGameObjectByType(TYPE_AGENT8);
	std::vector<int> vMeteors = Play::CollectGameObjectIDsByType(TYPE_METEOR);

	for (int id : vMeteors)
	{
		GameObject& obj_meteor = Play::GetGameObject(id);
		Play::UpdateGameObject(obj_meteor);

		if (gameState.agentState == STATE_FLYING && Play::IsColliding(obj_meteor, obj_agent8))
		{
			Play::SetSprite(obj_agent8, "agent8_dead", 0.05f);
			Play::StopAudioLoop("music");
			Play::PlayAudio("combust");
			gameState.agentState = STATE_DEAD;
		}

		if (Play::IsLeavingDisplayArea(obj_meteor))
		{
			if (!Play::IsVisible(obj_meteor))
			{
				IfObjectOffScreen(obj_meteor, obj_meteor.pos.x, obj_meteor.pos.y);
			}
		}

		Play::DrawObjectRotated(obj_meteor);
	}
}

void UpdateGems()
{
	GameObject& obj_agent8 = Play::GetGameObjectByType(TYPE_AGENT8);
	GameObject& obj_asteroid_attached = Play::GetGameObjectByType(TYPE_ASTEROID_ATTACHED);
	std::vector<int> vGems = Play::CollectGameObjectIDsByType(TYPE_GEM);

	for (int id_gem : vGems)
	{
		GameObject& obj_gem = Play::GetGameObject(id_gem);
		bool collected = false;

		if (Play::IsColliding(obj_gem, obj_agent8))
		{
			if ((gameState.agentState == STATE_FLYING && obj_agent8.frame > 60) || gameState.agentState == STATE_ATTACHED)
			{
				if (gameState.gem_remaining - 1 == 0)
				{
					gameState.agentState = STATE_LEVEL_COMPLETE;
				}
				gameState.gem_remaining -= 1;
				Play::PlayAudio("reward");
				collected = true;
			}
        }
		
		Play::UpdateGameObject(obj_gem);
		Play::DrawObjectRotated(obj_gem);

		if (!Play::IsVisible(obj_gem))
		{
			obj_gem.pos = Point2f(Play::RandomRollRange(10, DISPLAY_WIDTH), Play::RandomRollRange(10, DISPLAY_HEIGHT));
		}

		if (collected)
		{
			Play::DestroyGameObject(id_gem);
		}
			
	}
}

void UpdateDestroyed()
{
	GameObject& obj_agent8 = Play::GetGameObjectByType(TYPE_AGENT8);
	std::vector<int> vDestroyed = Play::CollectGameObjectIDsByType(TYPE_DESTROYED);

	for (int id_destroyed : vDestroyed)
	{
		GameObject& obj_destroyed = Play::GetGameObject(id_destroyed);
		obj_destroyed.velocity = { 0,0 };

		//start of asteroid pieces code
		int id_piece1 = Play::CreateGameObject(TYPE_ASTEROID_PIECES, obj_agent8.pos, 0, "asteroid_pieces"); // Is there another way to write this more concisely?
		int id_piece3 = Play::CreateGameObject(TYPE_ASTEROID_PIECES, obj_agent8.pos, 0, "asteroid_pieces");
		int id_piece2 = Play::CreateGameObject(TYPE_ASTEROID_PIECES, obj_agent8.pos, 0, "asteroid_pieces");
		GameObject& obj_piece1 = Play::GetGameObject(id_piece1);
		GameObject& obj_piece2 = Play::GetGameObject(id_piece2);
		GameObject& obj_piece3 = Play::GetGameObject(id_piece3);
		obj_piece1.animSpeed = 0;
		obj_piece2.animSpeed = 0;
		obj_piece3.animSpeed = 0;
		obj_piece1.frame = 0;
		obj_piece2.frame = 1;
		obj_piece3.frame = 2;
		int pieces_speed = 8;
		Play::SetGameObjectDirection(obj_piece1, pieces_speed, 0 * PLAY_PI);
		Play::SetGameObjectDirection(obj_piece2, pieces_speed, (4.0 / 3.0) * PLAY_PI);
		Play::SetGameObjectDirection(obj_piece3, pieces_speed, (2.0 / 3.0) * PLAY_PI);
		//end of asteroid pieces code

		Play::UpdateGameObject(obj_destroyed);
		Play::DestroyGameObject(id_destroyed);
	}

	std::vector<int> vPieces = Play::CollectGameObjectIDsByType(TYPE_ASTEROID_PIECES);
	for (int id_pieces : vPieces)
	{
		GameObject& obj_piece = Play::GetGameObject(id_pieces);
		int id = Play::CreateGameObject(TYPE_PARTICLE, obj_piece.pos + Point2f{ Play::RandomRoll(16) - 8,Play::RandomRoll(16) - 8 }, 0, "particle");
		Play::UpdateGameObject(obj_piece);
		Play::DrawObjectRotated(obj_piece);

		if (!Play::IsVisible(obj_piece))
			Play::DestroyGameObject(id_pieces);
	}
}

void UpdateAgent8()  
{
	GameObject& obj_agent8 = Play::GetGameObjectByType(TYPE_AGENT8);
	GameObject& obj_asteroid = Play::GetGameObjectByType(TYPE_ASTEROID);

	switch (gameState.agentState)
	{
	case STATE_LEVEL_START:
		
		CreateAsteroids();
		CreateMeteors();
		obj_agent8.velocity = { 0,0 };
		obj_agent8.pos = { 0,0 };
		obj_agent8.acceleration = { 0,0 };
		gameState.agentState = STATE_LEVEL;
		break;

	case STATE_LEVEL:
		Play::DrawFontText("132px", " LEVEL: " + std::to_string(gameState.level), { DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2 }, Play::CENTRE);
		UpdatePlayerAttached();
		if (obj_agent8.frame >5)
			gameState.agentState = STATE_ATTACHED;
		break;

	case STATE_ATTACHED:
		UpdatePlayerAttached();
		break;

	case STATE_FLYING:
		UpdatePlayerFlying();
		break;

	case STATE_DEAD:
		obj_agent8.rotation = 0;
		obj_agent8.velocity = { 0,-5 };
		Play::DrawFontText("105px", "MISSION FAILED!", { DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2 }, Play::CENTRE);
		Play::DrawFontText("64px", "Press ENTER to Restart", { DISPLAY_WIDTH / 2, DISPLAY_HEIGHT - 30 }, Play::CENTRE);

		if (Play::KeyPressed(VK_RETURN) == true)
		{
			obj_agent8.pos = { DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2 };
			gameState.level = 1;
			gameState.gem_remaining = 2;
			Play::StartAudioLoop("music");
			gameState.agentState = STATE_LEVEL_START; 
			for (int id_obj : Play::CollectGameObjectIDsByType(TYPE_ASTEROID))
				Play::DestroyGameObject(id_obj);
			for (int id_obj : Play::CollectGameObjectIDsByType(TYPE_METEOR))
				Play::DestroyGameObject(id_obj);
			for (int id_obj : Play::CollectGameObjectIDsByType(TYPE_GEM))
				Play::DestroyGameObject(id_obj);
		}
		break;

	case STATE_LEVEL_COMPLETE:
		UpdatePlayerFlying();
		Play::DrawFontText("105px", "LEVEL COMPLETE", { DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2 }, Play::CENTRE);
		Play::DrawFontText("64px", "Press ENTER to Continue", { DISPLAY_WIDTH / 2, DISPLAY_HEIGHT - 30 }, Play::CENTRE);

		for (int id_obj : Play::CollectGameObjectIDsByType(TYPE_METEOR)) 
			Play::DestroyGameObject(id_obj);

		if (Play::KeyPressed(VK_RETURN) == true)
		{
			gameState.level += 1;
			gameState.gem_remaining = gameState.level + 1;
			gameState.agentState = STATE_LEVEL_START; 
		}
		break;
	}
	Play::UpdateGameObject(obj_agent8);
	
}

void IfObjectOffScreen(GameObject& object, float& x, float& y)
{
	if (x < 0 || x > DISPLAY_WIDTH)
		object.pos = { DISPLAY_WIDTH - x, y };
	if (y < 0 || y > DISPLAY_HEIGHT)
		object.pos = { x, DISPLAY_HEIGHT - y };
} 
