#include "raylib.h"
#include "raymath.h"
#include <cstring>
#define SUPPORT_TRACELOG
#define SUPPORT_TRACELOG_DEBUG
#include "utils.h"

#include "rlgl.h"

// Define axis parameters
const float axisLength = 5.0f;  // Length of each axisC
const float coneLength = 0.3f;  // Length of the cone part
const float coneRadius = 0.1f;  // Radius of the cone base
const float lineRadius = 0.02f; // Radius for the axis lines    
void drawAxis() {
    // Draw coordinate axes with cones
    // X axis (red)
    DrawCylinderEx(Vector3Zero(), (Vector3){axisLength, 0, 0}, lineRadius, lineRadius, 8, RED);
    DrawCylinderEx((Vector3){axisLength, 0, 0}, (Vector3){axisLength + coneLength, 0, 0}, coneRadius, 0.0f, 8, RED);

    // Y axis (green)
    DrawCylinderEx(Vector3Zero(), (Vector3){0, axisLength, 0}, lineRadius, lineRadius, 8, GREEN);
    DrawCylinderEx((Vector3){0, axisLength, 0}, (Vector3){0, axisLength + coneLength, 0}, coneRadius, 0.0f, 8, GREEN);

    // Z axis (blue)
    DrawCylinderEx(Vector3Zero(), (Vector3){0, 0, axisLength}, lineRadius, lineRadius, 8, BLUE);
    DrawCylinderEx((Vector3){0, 0, axisLength}, (Vector3){0, 0, axisLength + coneLength}, coneRadius, 0.0f, 8, BLUE);
}

#include "rt.h"

enum MoveDirection {
	NONE, FORWARD_MOVE, BACKWARD_MOVE, RIGHT_MOVE, LEFT_MOVE
};

// This is the convention for the cube starting position and orientation:

// T = TOP (face 1), F = FRONT (face 2), R = RIGHT (face 3) 
// K = BACK (face 4), L = LEFT (face 5), M = BOTTOM (face 6) 
//
// z <-----------------------------------+
//          +---+             +---+      | 
//          | K |             | 4 |      | 
//      +---+---+---+     +---+---+---+  | 
//      | L | T | R |     | 5 | 1 | 3 |  | 
//      +---+---+---+     +---+---+---+  | 
//          | F |             | 2 |      | 
//          +---+             +---+      | 
//          | M |             | 6 |      | 
//          +---+             +---+      | 
//                                       |
//                                       V
//                                       x

struct Face {
	enum Number {
		NONE=0, TOP=1, FRONT=2, RIGHT=3, BACK=4, LEFT=5, BOTTOM=6
	};
	enum Orientation {
		ORIENTED_UP, ORIENTED_RIGHT, ORIENTED_DOWN, ORIENTED_LEFT
	};
	Number number;
	Orientation orientation;
};


// ORIENTED_UP      ORIENTED_RIGHT     ORIENTED_DOWN      ORIENTED_LEFT
// z <--------+     z <--------+       z <--------+       z <--------+
//      |  ^  |          |     |            |     |            |     |
//      |  |  |          | --> |            |  |  |            | <-- |
//      |     |          |     |            |  V  |            |     |
//      +-----+          +-----+            +-----+            +-----+
//            |                |                  |                  |
//            V                V                  V                  V
//            x                x                  x                  x
struct RotationInfo {
	char state[32];
	char tg_face[16];
	char ls_face[16];
	RotationEntry entry;
};


struct Cube {
	Model model;
	Vector3 position;
	Matrix transforms; // Transformations (translate, rotation and scale)
	Matrix rotations;
	MoveDirection moveDirection;
	const Vector3 X_VECTOR_POSITIVE = { 1, 0, 0 };
	const Vector3 X_VECTOR_NEGATIVE = { -1, 0, 0 };
	const Vector3 Z_VECTOR_POSITIVE = { 0, 0, 1 };
	const Vector3 Z_VECTOR_NEGATIVE = { 0, 0, -1 };

	float animationProgress = 0.0f;
	float smooth;
	
	bool isSliding;
	float slideSpeed;
	Vector3 slideStep;
	Vector3 startPosition;
	Vector3 endPosition;

	bool isRolling;
	float rollingTime;
	Vector3 rotationAxisV;
	rotation_axis rotationAxis;
	Vector3 rotationPivot;
	float rotationAngle;
	
	void init(Vector3 pos, bool smooth = true);
	void update(float);
	void updateSliding(float);
	void updateRolling(float);
	void draw();
	
	Sound rollWav;
	Sound slideWav;
	
	Texture2D numbersTex;
	Texture2D defaultTex;
	bool useNumbersTex = false;
	void swapTexture();
	
	Texture2D wordsTex;
	Image wordsImage;
	void drawWordsTex();

	RotationInfo rotationInfo;
};
Cube cube;

void Cube::init(Vector3 initPos, bool smoothBehaviour) {

	model = LoadModel("assets/cube-of-the-words.gltf"); // model cube is size 2x2

	wordsImage = LoadImage("./assets/cube-plain.png");
	wordsTex = LoadTextureFromImage(wordsImage);
	for (int i=0; i < model.materialCount; i++) {
		model.materials[i].maps[MATERIAL_MAP_ALBEDO].texture = wordsTex;
	}
	numbersTex = LoadTexture("./assets/cube-numbers.png");
	
	position = initPos; // it is moved 1.0f in y so it stays on the floor
	transforms = MatrixIdentity();
	// Matrix scale = MatrixScale(0.5, 0.5, 0.5);
	// transforms = MatrixMultiply(transforms, scale);
	
	isSliding = false;
	slideSpeed = 2.5f;
	
	isRolling = false;
	rollingTime = 0.5f;
	rotations = MatrixIdentity(); // cube has no rotations at start
	
	animationProgress = 0.0f;
	smooth = smoothBehaviour;
	
	rollWav = LoadSound("assets/roll.wav");
	slideWav = LoadSound("assets/slide.wav");

	init_state_map();
	strcpy(rotationInfo.state, "i");
	strcpy(rotationInfo.ls_face, "1U");
	strcpy(rotationInfo.tg_face, "6D");
}

void Cube::swapTexture() {
	useNumbersTex = !useNumbersTex;
	for (int i=0; i < model.materialCount; i++) {
		model.materials[i].maps[MATERIAL_MAP_ALBEDO].texture = useNumbersTex ? numbersTex : wordsTex;
	}
}

void Cube::drawWordsTex() {
	DrawTextureV(useNumbersTex ? numbersTex : wordsTex, {20, 460}, WHITE);
}


void Cube::updateSliding(float delta) {
	
	MoveDirection inputDirection = 
		IsKeyPressed(KEY_UP) ? FORWARD_MOVE :
		IsKeyPressed(KEY_DOWN) ? BACKWARD_MOVE :
		IsKeyPressed(KEY_LEFT) ? LEFT_MOVE :
		IsKeyPressed(KEY_RIGHT) ? RIGHT_MOVE :
		NONE;

	if (!isSliding && !isRolling && inputDirection != NONE) {

		if (inputDirection == FORWARD_MOVE) {
			slideStep = {0, 0, -2};
		}
		else if (inputDirection == BACKWARD_MOVE) {
			slideStep = {0, 0, 2};
		}		
		else if (inputDirection == LEFT_MOVE) {
			slideStep = {-2, 0, 0};
		}		
		else if (inputDirection == RIGHT_MOVE) {
			slideStep = {2, 0, 0};
		}
		
		moveDirection = inputDirection;
		startPosition = position;
		endPosition = Vector3Add(position, slideStep);
		
		isSliding = true;
		PlaySound(slideWav);
	}

	if (isSliding) {
		
		animationProgress += delta * slideSpeed;

		// Use smooth easing for animation
		float t = animationProgress;
		float smoothT = t * t * (3.0f - 2.0f * t); // Smoothstep formula
		
		if (smooth) {
			position = Vector3Lerp(startPosition, endPosition, smoothT);
		} else {
			position = Vector3Lerp(startPosition, endPosition, t);
		}

		Matrix translationToOrigin = MatrixTranslate(-position.x, -position.y, -position.z);
		Matrix translationBackFromOrigin = MatrixTranslate(position.x, position.y, position.z);
		transforms = MatrixMultiply(translationToOrigin, rotations);
		transforms = MatrixMultiply(transforms, translationBackFromOrigin);
		
		if (animationProgress >= 1.0f) {
			position = endPosition;
			isSliding = false;
			animationProgress = 0.0f;			
		}
	}
}


void Cube::updateRolling(float delta) {

	MoveDirection inputDirection = 
		IsKeyPressed(KEY_W) ? FORWARD_MOVE :
		IsKeyPressed(KEY_A) ? LEFT_MOVE :
		IsKeyPressed(KEY_S) ? BACKWARD_MOVE :
		IsKeyPressed(KEY_D) ? RIGHT_MOVE :
		NONE;

	if (!isRolling && !isSliding && inputDirection != NONE) {

		Vector3 offset; // offset for rotationOrigin
		Vector3 translation;
		
		if (inputDirection == FORWARD_MOVE) {
			offset = {0, -1, -1};
			rotationAxisV = X_VECTOR_NEGATIVE;
			rotationAxis = X_NEGATIVE;
			translation = {0, 0, -2};
		}
		else if (inputDirection == BACKWARD_MOVE) {
			offset = {0, -1, 1};
			rotationAxisV = X_VECTOR_POSITIVE;
			rotationAxis = X_POSITIVE;
			translation = {0, 0, 2};
		}		
		else if (inputDirection == LEFT_MOVE) {
			offset = {-1, -1, 0};
			rotationAxisV = Z_VECTOR_POSITIVE;
			rotationAxis = Z_POSITIVE;
			translation = {-2, 0, 0};
		}		
		else if (inputDirection == RIGHT_MOVE) {
			offset = {1, -1, 0};
			rotationAxisV = Z_VECTOR_NEGATIVE;
			rotationAxis = Z_NEGATIVE;
			translation = {2, 0, 0};
		}
		
		moveDirection = inputDirection;

		rotationPivot = Vector3Add(position, offset);
		endPosition = Vector3Add(position, translation);
		isRolling = true;
	}

	if (isRolling) {
		
		animationProgress += delta / rollingTime;
		
		float t = animationProgress;
		float smoothT = t * t * (3.0f - 2.0f * t); // Smoothstep formula

		if (smooth) {
			rotationAngle = Lerp(0.0f, 90.0f, smoothT);
		} else {
			rotationAngle = Lerp(0.0f, 90.0f, t);
		}
		
		// The cube can have suffered some rotations, so it is likely not to be in the initial rotation state.
		// To proceed with the next rotation, first it is needed to reproduce its rotation state.
		
		// So, for a cube model translated in current position but with a coordinate system not rotated:
		// 1) it has to be translated to the origin
		// 2) rotated with the stored rotations and 
		// 3) translated back.
		// This is achieved with these matrix multiplications
		Matrix translationToOrigin = MatrixTranslate(-position.x, -position.y, -position.z);
		Matrix translationBackFromOrigin = MatrixTranslate(position.x, position.y, position.z);
		transforms = MatrixMultiply(translationToOrigin, rotations);
		transforms = MatrixMultiply(transforms, translationBackFromOrigin);
		
		// Now the current rotation is applied using rotationPivot, rotationAxisV and the changing rotationAngle
		// Combining the matrices: first translate to rotationPivot, then rotating, then translate back
		Matrix translationToPivot = MatrixTranslate(-rotationPivot.x, -rotationPivot.y, -rotationPivot.z);
		Matrix rotation = MatrixRotate(rotationAxisV, rotationAngle * DEG2RAD);
		Matrix translationBackFromPivot = MatrixTranslate(rotationPivot.x, rotationPivot.y, rotationPivot.z);
		
		Matrix translationPlusRotation = MatrixMultiply(translationToPivot, rotation);
		transforms = MatrixMultiply(transforms, translationPlusRotation);
		transforms = MatrixMultiply(transforms, translationBackFromPivot);
		
		// Matrix scale = MatrixScale(0.5, 0.5, 0.5);
		// transforms = MatrixMultiply(transforms, scale);

		if (animationProgress >= 1.0f) {

			position = endPosition;
			isRolling = false;
			animationProgress = 0.0f;

			// Add the finished rotation (always 90 degrees) to accumulated rotations
			Matrix finishedRotation = MatrixRotate(rotationAxisV, 90.0f * DEG2RAD);
			rotations = MatrixMultiply(rotations, finishedRotation);
			
			Matrix translationToOrigin = MatrixTranslate(-position.x, -position.y, -position.z);
			Matrix translationBackFromOrigin = MatrixTranslate(position.x, position.y, position.z);
			transforms = MatrixMultiply(translationToOrigin, rotations);
			transforms = MatrixMultiply(transforms, translationBackFromOrigin);
			
			
			get_next_rotation_state(rotationInfo.state, rotationAxis, &rotationInfo.entry);
			strcpy(rotationInfo.state, rotationInfo.entry.state);
			strcpy(rotationInfo.ls_face, rotationInfo.entry.ls_face);
			strcpy(rotationInfo.tg_face, rotationInfo.entry.tg_face);
			
			PlaySound(rollWav);
		}
	}
}

void Cube::update(float delta) {

	updateSliding(delta);
	updateRolling(delta);
}

void Cube::draw() {
	
	rlPushMatrix();
	rlMultMatrixf(MatrixToFloat(transforms));
	DrawModel(model, position, 1.0f, WHITE);
	rlPopMatrix();
}

bool cameraUpdateEnabled = false;
Camera3D camera;

Camera3D poseCamera;
RenderTexture poseRenderTexture;
Rectangle poseRect = { 0.0f, 0.0f, 300, -168 };

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

void DrawScaledText(const char* text, int x, int y, int fontSize, Color color) {
    float scaleX = (float)GetScreenWidth() / SCREEN_WIDTH;
    float scaleY = (float)GetScreenHeight() / SCREEN_HEIGHT;
    float scale = fminf(scaleX, scaleY); // keep aspect ratio

    DrawText(text, x * scale, y * scale, fontSize * scale, color);
}

Font fontBubble;
struct Word {
	Model model;
	Texture2D texture;
	
	void init(const char* text) {
		Image image = GenImageColor(120, 120, BLANK);
		ImageDrawTextEx(&image, fontBubble, text, {25, 45}, (float) fontBubble.baseSize/2, 0.0f, BLACK);
		texture = LoadTextureFromImage(image);
		
		Mesh planeMesh = GenMeshPlane(2.0, 2.0, 1, 1);
		model = LoadModelFromMesh(planeMesh);
		for (int i=0; i<model.materialCount; i++) {
			model.materials[i].maps[MATERIAL_MAP_DIFFUSE].texture = texture;
			model.materials[i].maps[MATERIAL_MAP_ALBEDO].texture = texture;
		}
	}
};

void drawText() {
	DrawFPS(10, 10);
	
	DrawScaledText(TextFormat("Q - toggle camera control: %s", cameraUpdateEnabled ? "ON" : "OFF"),
				   10, 30, 20, BLUE);
	DrawScaledText(TextFormat("WASD & Arrows - control %s", cameraUpdateEnabled ? "camera" : "cube"),
				   10, 50, 20, BLUE);
	DrawScaledText(TextFormat("E - toggle cube numbers/words texture"),
				   10, 80, 20, BLUE);
	DrawScaledText(TextFormat("T - toggle hide 2D texture"),
				   10, 100, 20, BLUE);
	
	if (!cameraUpdateEnabled) {
		
		DrawScaledText("Movement data", 10, 150, 20, RED);		
		DrawScaledText(TextFormat("position: (%.2f, %.2f, %.2f)", cube.position.x, cube.position.y, cube.position.z), 
					   10, 170, 20, DARKGRAY);
		DrawScaledText(TextFormat("endPosition: (%.2f, %.2f, %.2f)", 
								  cube.endPosition.x, cube.endPosition.y, cube.endPosition.z), 
					   10, 190, 20, DARKGRAY);
		DrawScaledText(TextFormat("moveDirection: %s", 
								  cube.moveDirection == FORWARD_MOVE ? "forward" :
								  cube.moveDirection == BACKWARD_MOVE ? "backward" :
								  cube.moveDirection == RIGHT_MOVE ? "right" :
								  cube.moveDirection == LEFT_MOVE ? "left" : "idle"),
					   10, 210, 20, DARKGRAY);

		
		DrawScaledText("Arrows slide the cube", 10, 240, 20, RED);
		DrawScaledText(TextFormat("isSliding: %s", cube.isSliding ? "true" : "false"),
					   10, 260, 20, DARKGRAY);
		DrawScaledText(TextFormat("slideStep: (%.1f, %.1f, %.1f)", cube.slideStep.x, cube.slideStep.y, cube.slideStep.z), 
					   10, 280, 20, DARKGRAY);

		
		
		DrawScaledText("WASD rolls the cube", 10, 320, 20, RED);
		DrawScaledText(TextFormat("isRolling: %s", cube.isRolling ? "true" : "false"),
					   10, 340, 20, DARKGRAY);		
		DrawScaledText(TextFormat("rotationAngle: %.2f", cube.rotationAngle),
					   10, 360, 20, DARKGRAY);

		
		DrawScaledText("ROTATION", 10, 400, 20, GREEN);
		DrawScaledText(TextFormat("TOP:%s | BOTTOM:%s | state:%s", cube.rotationInfo.ls_face, 
								  cube.rotationInfo.tg_face, cube.rotationInfo.state),
					   10, 420, 20, BLUE);
		
		DrawScaledText(TextFormat("animationProgress: %.2f", cube.animationProgress),
					   1000, 680, 20, DARKGRAY);
	}
}


Font fontKaisg;
bool showFont = false;
bool showWordsTex = true;

Word w_Cube;

void updateFrameWindow() {
	
	float delta = GetFrameTime();

	if (IsKeyPressed(KEY_Q)) {
		cameraUpdateEnabled = !cameraUpdateEnabled;
		if (cameraUpdateEnabled) {
			SetMousePosition(GetScreenWidth() /2, GetScreenHeight() /2);
		}
	}
	
	if (IsKeyPressed(KEY_E)) {
		cube.swapTexture();
	}

	if (IsKeyPressed(KEY_T)) {
		showWordsTex = !showWordsTex;
	}
	
	if (IsKeyPressed(KEY_F)) {
		showFont = !showFont;
	}
	
	if (cameraUpdateEnabled) {
		UpdateCamera(&camera, CAMERA_THIRD_PERSON);
	} else {
		cube.update(delta);
	}
	
	
	BeginDrawing();
	ClearBackground(RAYWHITE);
	BeginMode3D(camera);
	{
		cube.draw();
		
		DrawModel(w_Cube.model, {1, 0, 1}, 1.0, WHITE);
		
		DrawGrid(10, 2.0f);
		drawAxis();
	}
	EndMode3D();

	
	if (showFont) {
		DrawTexture(fontBubble.texture, SCREEN_WIDTH/2 - fontKaisg.texture.width/2, 50, BLUE);
	} 

	if (showWordsTex) {
		cube.drawWordsTex();
	}
	
	// Draw up-right BLUE cube with axis with renderTexture
	DrawTextureRec(poseRenderTexture.texture, poseRect, (Vector2){960.0f, 20.0f}, WHITE);
	DrawRectangleLines(960, 20, 300, 168, GREEN);
	
	drawText();
	EndDrawing();
}

#ifdef PLATFORM_WEB
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>

EM_BOOL ResizeCallback(int eventType, const EmscriptenUiEvent *e, void *userData) {
    double w, h;
    emscripten_get_element_css_size("#canvas", &w, &h);
    SetWindowSize((int)w, (int)h);
    return EM_TRUE;
}
#endif

void buildTexturePose() {
	BeginTextureMode(poseRenderTexture);
	ClearBackground(RAYWHITE);
	BeginMode3D(poseCamera);
	{
		DrawCube({0,0,0}, 1.0, 1.0, 1.0, SKYBLUE);
		DrawCubeWires({0,0,0}, 1.0, 1.0, 1.0, GREEN);

		drawAxis();
	}
	EndMode3D();
	EndTextureMode();
}

int main(void)
{
	SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Cube Of The Words!");
	
	SetTraceLogLevel(LOG_ALL);
	SetTargetFPS(60);
	
	InitAudioDevice();
	
	camera.position = (Vector3){ 3.0f, 10.0f, 12.0f };
	camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
	camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
	camera.fovy = 45.0f;
	camera.projection = CAMERA_PERSPECTIVE;

	cube.init({5, 1, 1}, true);
	
	poseCamera.position = (Vector3){ 1.5f, 1.5f, 1.5f };
	poseCamera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
	poseCamera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
	poseCamera.fovy = 45.0f;
	poseCamera.projection = CAMERA_PERSPECTIVE;
	poseRenderTexture = LoadRenderTexture(300, 168);
	buildTexturePose();

	
	fontBubble = LoadFontEx("./assets/Game Bubble.ttf", 64, 0, 0);
	fontKaisg = LoadFontEx("./assets/KAISG.ttf", 64, 0, 0);
	
	
	w_Cube.init("Cube");
	
	
#ifdef PLATFORM_WEB
#ifdef RESIZE_CALLBACK
	emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, EM_TRUE, ResizeCallback);
	ResizeCallback(0, NULL, NULL);
#endif
	emscripten_set_main_loop(updateFrameWindow, 0, 1);	
#else
	while (!WindowShouldClose()) {
		updateFrameWindow();
	}
	CloseWindow();
#endif		
    
	return 0;
}
