#include <cstdio>
#include <string>
#include <vector>
#include <algorithm>

#include "raylib.h"

enum class ObjectAnchor {
	TOP_LEFT,
	TOP_CENTER,
	TOP_RIGHT,
	BOTTOM_LEFT,
	BOTTOM_RIGHT,
	BOTTOM_CENTER,
	CENTER_LEFT,
	CENTER_CENTER,
	CENTER_RIGHT
};

struct GameObject {
	float x, y;
	int z;
	float absolute_x, absolute_y;
	int absolute_z;
	float width, height;
	Texture2D tex;
	bool draggable;
	bool dragging;
	std::string text;
	int font_size;
	bool drawable;
	ObjectAnchor anchor;
	std::vector<GameObject*> children;
	GameObject* parent;
};

bool is_mouse_dragging_something = false;
std::vector<GameObject*> drawables;

float ElementCenter(float element, float container)
{
	return (container / 2.0f) - (element / 2.0f);
}

Vector2 ElementCenterXY(Vector2 element, Vector2 container)
{
	return { ElementCenter(element.x, container.x), ElementCenter(element.y, container.y) };
}

void DrawTextureCentered(Vector2 element, Vector2 container, Texture2D tex)
{
	DrawTextureV(tex, ElementCenterXY(element, container), WHITE);
}

GameObject ObjectInitDefault(Vector2 pos, Vector2 dimensions, Texture2D tex, const std::string& text, int font_size)
{
	GameObject obj = {};
	obj.tex = tex;
	obj.text = text;
	obj.font_size = font_size;
	obj.dragging = false;
	obj.x = pos.x;
	obj.y = pos.y;
	obj.parent = nullptr;

	if (obj.tex.width != 0 || !obj.text.empty()) {
		obj.drawable = true;
	}
	if (obj.tex.width != 0) {
		obj.width = (float)tex.width;
		obj.height = (float)tex.height;
	}
	if (dimensions.x != 0.0f && dimensions.y != 0.0f) {
		obj.width = dimensions.x;
		obj.height = dimensions.y;
	}

	return obj;
}

GameObject ObjectInitWithTextureString(Vector2 pos, const std::string& tex_path)
{
	Texture2D tex = {};
	if (!tex_path.empty()) {
		tex = LoadTexture(tex_path.c_str());
	}
	return ObjectInitDefault(pos, {0, 0}, tex, "", 0);
}

GameObject ObjectInitWithDimensions(Vector2 pos, Vector2 dimensions)
{
	Texture2D tex = {};
	return ObjectInitDefault(pos, dimensions, tex, "", 0);
}

GameObject ObjectInitText(Vector2 pos, const std::string& text, int font_size)
{
	Vector2 dimensions = { (float)MeasureText(text.c_str(), font_size), (float)font_size };
	Texture2D tex = {};
	return ObjectInitDefault(pos, dimensions, tex, text, font_size);
}

void ObjectAddChild(GameObject& parent, GameObject& child)
{
	if(child.parent != nullptr) return;
	parent.children.push_back(&child);
	child.parent = &parent;
}

Vector2 ObjectCalculatePosition(GameObject& obj)
{
	Vector2 anchor_result = {};

	if (obj.parent != nullptr) {
		Vector2 parent_position = { obj.parent->absolute_x, obj.parent->absolute_y };

		switch (obj.anchor) {
			case ObjectAnchor::TOP_LEFT:
				anchor_result = { obj.x + parent_position.x, obj.y + parent_position.y };
				break;
			case ObjectAnchor::TOP_RIGHT:
				anchor_result = { (parent_position.x + obj.parent->width) - obj.width + obj.x, obj.y + parent_position.y };
				break;
			case ObjectAnchor::TOP_CENTER:
				anchor_result = { ElementCenter(obj.width, obj.parent->width) + parent_position.x + obj.x, obj.y + parent_position.y };
				break;
			case ObjectAnchor::BOTTOM_LEFT:
				anchor_result = { obj.x + parent_position.x, (parent_position.y + obj.parent->height) - obj.height + obj.y };
				break;
			case ObjectAnchor::BOTTOM_RIGHT:
				anchor_result = { (parent_position.x + obj.parent->width) - obj.width + obj.x, (parent_position.y + obj.parent->height) - obj.height + obj.y };
				break;
			case ObjectAnchor::BOTTOM_CENTER:
				anchor_result = { ElementCenter(obj.width, obj.parent->width) + parent_position.x + obj.x, (parent_position.y + obj.parent->height) - obj.height + obj.y };
				break;
			case ObjectAnchor::CENTER_LEFT:
				anchor_result = { obj.x + parent_position.x, ElementCenter(obj.height, obj.parent->height) + parent_position.y + obj.y };
				break;
			case ObjectAnchor::CENTER_CENTER:
				anchor_result = { ElementCenter(obj.width, obj.parent->width) + parent_position.x + obj.x, ElementCenter(obj.height, obj.parent->height) + parent_position.y + obj.y };
				break;
			case ObjectAnchor::CENTER_RIGHT:
				anchor_result = { (parent_position.x + obj.parent->width) - obj.width + obj.x, ElementCenter(obj.height, obj.parent->height) + parent_position.y + obj.y };
				break;
		}
	} else {
		anchor_result = { obj.x, obj.y };
	}

	obj.absolute_x = anchor_result.x;
	obj.absolute_y = anchor_result.y;
	return anchor_result;
}

// TODO: maybe this will be more adequate if its called UpdateAllObjectRelativeProperties? Because its not just updating positions, but also z-indices etc.
void UpdateAllObjectPositions(GameObject& obj)
{
	ObjectCalculatePosition(obj);
	if (obj.parent != nullptr) obj.absolute_z = obj.z + obj.parent->z; 
	for (GameObject* child : obj.children) {
		UpdateAllObjectPositions(*child);
	}
}

void DrawAllObjects()
{
	for (GameObject* drawable : drawables) {
		if (drawable->drawable) {
			if (drawable->tex.width != 0) {
				DrawTextureEx(drawable->tex, Vector2{drawable->absolute_x, drawable->absolute_y}, 0.0f, 1.0f, WHITE);
			}
			if (drawable->font_size > 0) {
				DrawText(drawable->text.c_str(), (int)drawable->absolute_x, (int)drawable->absolute_y, drawable->font_size, BLACK);
			}
		}
	}
}

Vector2 GetScreenDimensions()
{
	return { (float)GetScreenWidth(), (float)GetScreenHeight() };
}

int main()
{
	SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_HIGHDPI);
	InitWindow(1366, 768, "Document Game");
	SetTargetFPS(GetMonitorRefreshRate(GetCurrentMonitor()));


	GameObject root = ObjectInitWithDimensions({0, 0}, GetScreenDimensions());

	GameObject paper = ObjectInitWithTextureString({0, 0}, "assets/paper.png");
	paper.anchor = ObjectAnchor::CENTER_CENTER;
	paper.draggable = true;
	GameObject person_name = ObjectInitText({0, 50}, "Person name", 16);
	person_name.anchor = ObjectAnchor::TOP_CENTER;
	ObjectAddChild(root, paper);
	ObjectAddChild(paper, person_name);
	drawables.push_back(&paper);
	drawables.push_back(&person_name);

	GameObject paper2 = ObjectInitWithTextureString({0, 0}, "assets/paper.png");
	ObjectAddChild(root, paper2);
	GameObject person_name2 = ObjectInitText({0, 50}, "Testando papel 2", 16);
	person_name2.anchor = ObjectAnchor::TOP_CENTER;
	ObjectAddChild(paper2, person_name2);
	drawables.push_back(&paper2);
	drawables.push_back(&person_name2);

	while (!WindowShouldClose()) {
		BeginDrawing();

		ClearBackground(RAYWHITE);
		DrawFPS(10, 10);

		std::sort(drawables.begin(), drawables.end(), [](const GameObject* a, const GameObject*b) {
			return a->absolute_z < b->absolute_z;
		});

		UpdateAllObjectPositions(root);
		DrawAllObjects();

		if (IsKeyPressed(KEY_Q))      paper.anchor = ObjectAnchor::TOP_LEFT;
		else if (IsKeyPressed(KEY_W)) paper.anchor = ObjectAnchor::TOP_CENTER;
		else if (IsKeyPressed(KEY_E)) paper.anchor = ObjectAnchor::TOP_RIGHT;
		else if (IsKeyPressed(KEY_A)) paper.anchor = ObjectAnchor::CENTER_LEFT;
		else if (IsKeyPressed(KEY_S)) paper.anchor = ObjectAnchor::CENTER_CENTER;
		else if (IsKeyPressed(KEY_D)) paper.anchor = ObjectAnchor::CENTER_RIGHT;
		else if (IsKeyPressed(KEY_Z)) paper.anchor = ObjectAnchor::BOTTOM_LEFT;
		else if (IsKeyPressed(KEY_X)) paper.anchor = ObjectAnchor::BOTTOM_CENTER;
		else if (IsKeyPressed(KEY_C)) paper.anchor = ObjectAnchor::BOTTOM_RIGHT;

		if(!is_mouse_dragging_something && IsMouseButtonDown(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), Rectangle {paper.absolute_x, paper.absolute_y, paper.width, paper.height})) {
			paper.dragging = true;
			is_mouse_dragging_something = true;
		}
		if(!is_mouse_dragging_something && IsMouseButtonDown(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), Rectangle {paper2.absolute_x, paper2.absolute_y, paper2.width, paper2.height})) {
			paper2.dragging = true;
			is_mouse_dragging_something = true;
		}

		if(paper.dragging) {
			Vector2 delta = GetMouseDelta();
			paper.x += delta.x;
			paper.y += delta.y;

			paper.z = 4;
			DrawText(TextFormat("paper.x = %.2f", paper.x), 10, 30, 24, BLACK);
			DrawText(TextFormat("paper.y = %.2f", paper.y), 10, 54, 24, BLACK);

			if(IsMouseButtonUp(MOUSE_BUTTON_LEFT)) {
				is_mouse_dragging_something = false;
				paper.dragging = false;
				paper.z = 3;
			}
		}

		if(paper2.dragging) {
			Vector2 delta = GetMouseDelta();
			paper2.x += delta.x;
			paper2.y += delta.y;

			paper2.z = 40;
			DrawText(TextFormat("paper2.x = %.2f", paper2.x), 10, 30, 24, BLACK);
			DrawText(TextFormat("paper2.y = %.2f", paper2.y), 10, 54, 24, BLACK);
			DrawText(TextFormat("paper2.z = %d", paper2.z), 10, 78, 24, BLACK);
			DrawText(TextFormat("paper2.absolute_z = %d", paper2.absolute_z), 10, 102, 24, BLACK);

			if(IsMouseButtonUp(MOUSE_BUTTON_LEFT)) {
				is_mouse_dragging_something = false;
				paper2.dragging = false;
				paper2.z = 3;
			}
		}

		EndDrawing();
	}

	CloseWindow();
	return 0;
}
