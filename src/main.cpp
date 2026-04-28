#include <cstdio>
#include <string>
#include <vector>

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
    float absolute_x, absolute_y;
    float width, height;
    Texture2D tex;
    bool dragging;
    std::string text;
    int font_size;
    bool drawable;
    ObjectAnchor anchor;
    std::vector<GameObject*> children;
    GameObject* parent;
};

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
            anchor_result = { (parent_position.x + obj.parent->width) - obj.width - obj.x, obj.y + parent_position.y };
            break;
        case ObjectAnchor::TOP_CENTER:
            anchor_result = { ElementCenter(obj.width, obj.parent->width) + parent_position.x, obj.y + parent_position.y };
            break;
        case ObjectAnchor::BOTTOM_LEFT:
            anchor_result = { obj.x + parent_position.x, (parent_position.y + obj.parent->height) - obj.height - obj.y };
            break;
        case ObjectAnchor::BOTTOM_RIGHT:
            anchor_result = { (parent_position.x + obj.parent->width) - obj.width - obj.x, (parent_position.y + obj.parent->height) - obj.height - obj.y };
            break;
        case ObjectAnchor::BOTTOM_CENTER:
            anchor_result = { ElementCenter(obj.width, obj.parent->width) + parent_position.x, (parent_position.y + obj.parent->height) - obj.height - obj.y };
            break;
        case ObjectAnchor::CENTER_LEFT:
            anchor_result = { obj.x + parent_position.x, ElementCenter(obj.height, obj.parent->height) + parent_position.y };
            break;
        case ObjectAnchor::CENTER_CENTER:
            anchor_result = { ElementCenter(obj.width, obj.parent->width) + parent_position.x, ElementCenter(obj.height, obj.parent->height) + parent_position.y };
            break;
        case ObjectAnchor::CENTER_RIGHT:
            anchor_result = { (parent_position.x + obj.parent->width) - obj.width - obj.x, ElementCenter(obj.height, obj.parent->height) + parent_position.y };
            break;
        }
    } else {
        anchor_result = { obj.x, obj.y };
    }

    obj.absolute_x = anchor_result.x;
    obj.absolute_y = anchor_result.y;
    return anchor_result;
}

void ObjectDraw(GameObject& obj)
{
    if (obj.drawable) {
        Vector2 object_position = ObjectCalculatePosition(obj);
        if (obj.tex.width != 0) {
            DrawTextureEx(obj.tex, object_position, 0.0f, 1.0f, WHITE);
        }
        if (obj.font_size > 0) {
            DrawText(obj.text.c_str(), (int)object_position.x, (int)object_position.y, obj.font_size, BLACK);
        }
    }
    for (GameObject* child : obj.children) {
        ObjectDraw(*child);
    }
}

Vector2 GetScreenDimensions()
{
    return { (float)GetScreenWidth(), (float)GetScreenHeight() };
}

int main()
{
    InitWindow(1600, 900, "Document Game");

    GameObject root = ObjectInitWithDimensions({0, 0}, GetScreenDimensions());
    GameObject paper = ObjectInitWithTextureString({0, 0}, "assets/paper.png");
    paper.anchor = ObjectAnchor::CENTER_CENTER;

    GameObject person_name = ObjectInitText({0, 50}, "Person name", 16);
    person_name.anchor = ObjectAnchor::TOP_CENTER;

    ObjectAddChild(root, paper);

    std::vector<GameObject> texts(1024);
    texts[0] = ObjectInitText({0, 0}, std::to_string(0), 16);
    texts[0].anchor = ObjectAnchor::TOP_LEFT;
    ObjectAddChild(paper, texts[0]);

    for (int i = 1; i < 1024; i++) {
        texts[i] = ObjectInitText({0, (float)(i + 1) * 1.0f}, std::to_string(i), 16);
        texts[i].anchor = ObjectAnchor::TOP_LEFT;
        ObjectAddChild(texts[i - 1], texts[i]);
    }

    while (!WindowShouldClose()) {
        BeginDrawing();

        ClearBackground(RAYWHITE);
        DrawFPS(10, 10);

        ObjectDraw(root);

        if (IsKeyPressed(KEY_Q))      person_name.anchor = ObjectAnchor::TOP_LEFT;
        else if (IsKeyPressed(KEY_W)) person_name.anchor = ObjectAnchor::TOP_CENTER;
        else if (IsKeyPressed(KEY_E)) person_name.anchor = ObjectAnchor::TOP_RIGHT;
        else if (IsKeyPressed(KEY_A)) person_name.anchor = ObjectAnchor::CENTER_LEFT;
        else if (IsKeyPressed(KEY_S)) person_name.anchor = ObjectAnchor::CENTER_CENTER;
        else if (IsKeyPressed(KEY_D)) person_name.anchor = ObjectAnchor::CENTER_RIGHT;
        else if (IsKeyPressed(KEY_Z)) person_name.anchor = ObjectAnchor::BOTTOM_LEFT;
        else if (IsKeyPressed(KEY_X)) person_name.anchor = ObjectAnchor::BOTTOM_CENTER;
        else if (IsKeyPressed(KEY_C)) person_name.anchor = ObjectAnchor::BOTTOM_RIGHT;

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
