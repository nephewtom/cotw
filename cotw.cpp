#include "raylib.h"
#include "raymath.h"
#include <cstring>
#define SUPPORT_TRACELOG
#define SUPPORT_TRACELOG_DEBUG
#include "utils.h"

#include "rlgl.h"

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

// ---------------------------------------------------------------------------------------
// Words
// ---------------------------------------------------------------------------------------
Font fontBubble;
struct Word {
	Model model;
	Texture2D texture;
	Image wordImage;
	Image blankImage;
	
	void init(const char* text, Vector2 position) {
		
		// RLAPI void ImageDrawTextEx(Image *dst, Font font, const char *text, 
		//                            Vector2 position, float fontSize, float spacing, Color tint);
		blankImage = GenImageColor(120, 120, BLANK);
		
		wordImage = GenImageColor(120, 120, BLANK);
		ImageDrawTextEx(&wordImage, fontBubble, text, position, (float) fontBubble.baseSize/2, 0.0f, BLACK);
		
		texture = LoadTextureFromImage(wordImage);
		
		Mesh planeMesh = GenMeshPlane(2.0, 2.0, 1, 1);
		model = LoadModelFromMesh(planeMesh);
		for (int i=0; i<model.materialCount; i++) {
			model.materials[i].maps[MATERIAL_MAP_DIFFUSE].texture = texture;
			model.materials[i].maps[MATERIAL_MAP_ALBEDO].texture = texture;
		}
	}
	
	void pickup() {
		// the word disappears on the ground
		UpdateTexture(texture, blankImage.data);
	}
	
	void release() {
		// the word appears on the ground
		UpdateTexture(texture, wordImage.data);		
	}
};
Word wordCube;


// ---------------------------------------------------------------------------------------
// Cube stuff
// ---------------------------------------------------------------------------------------
#include "rt.h"

struct RotationInfo {
	char state[32];
	char tg_face[16];
	char ls_face[16];
	RotationEntry entry;	
};

enum MoveDirection {
	NONE, FORWARD_MOVE, BACKWARD_MOVE, RIGHT_MOVE, LEFT_MOVE
};

struct CellPos {
	int x;
	int z;
};

struct Cube {
	Model model;
	Vector3 position;
	Vector3 endPosition; // For movement
	
	// TODO: Shouldn't i use model.transform ?
	Matrix transforms; // Transformations (translate, rotation and scale)
	Matrix rotations;

	void transformToInitalState();
	
	MoveDirection moveDirection;
	const Vector3 X_VECTOR_POSITIVE = { 1, 0, 0 };
	const Vector3 X_VECTOR_NEGATIVE = { -1, 0, 0 };
	const Vector3 Z_VECTOR_POSITIVE = { 0, 0, 1 };
	const Vector3 Z_VECTOR_NEGATIVE = { 0, 0, -1 };

	float animationProgress = 0.0f;
	float smooth;
	
	bool isSliding = false;
	struct SlideAnim {
		float speed = 2.5f;
		Vector3 step;
		Vector3 start;
	};
	SlideAnim slide;

	bool isRolling = false;
	struct RollingAnim {
		float duration = 0.5f;
		Axis axis;
		Vector3 axisV;
		Vector3 pivot;
		float angle;
	};
	RollingAnim rollAnim;
	
	void init(Vector3 pos, bool smooth = true);
	void handleInput();
	void handleSlideInput();
	void handleRollInput();
	void handleWordsInput();
	
	void update(float);
	void updateSliding(float);
	void updateRolling(float);
	void draw();
	
	
	Texture2D numbersTex;
	Texture2D defaultTex;
	bool useNumbersTex = false;
	void swapTexture();
	
	Texture2D wordsTex;
	Image wordsImage;
	void drawWordsTex();
	RotationInfo rotationInfo;

	bool isSwappingWord = false; // isSwappingWord
	struct SwapAnim {
		float progress = 0.0f;
		float duration = 0.3f;
		float baseHeight = 2.0f;
		float currentHeight = baseHeight;
		float squashHeight = 1.2f;
		float scale = 1.0f;
		Axis axis;
	};
	SwapAnim swap;
	void setScaleAxis();
	
	bool isPickUp = false;
	CellPos cellPos;
	
	
	Sound rollWav;
	Sound slideWav;
	Sound pickupWav;
	Sound releaseWav;
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
	cellPos = { (int)position.x/2 + 1, (int)position.z/2 + 1};

	transforms = MatrixIdentity();
	
	rotations = MatrixIdentity(); // cube has no rotations at start
	
	smooth = smoothBehaviour;
	
	rollWav = LoadSound("assets/roll.wav");
	slideWav = LoadSound("assets/slide.wav");
	pickupWav = LoadSound("assets/pickup.wav");
	releaseWav = LoadSound("assets/release.wav");
	
	
	// Check rt.h to understand this
	init_state_map();
	strcpy(rotationInfo.state, "i");
	strcpy(rotationInfo.ls_face, "1U");
	strcpy(rotationInfo.tg_face, "6D");	
	swap.axis = Y_POSITIVE;
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

void Cube::handleInput() {
	handleSlideInput();
	handleRollInput();
	handleWordsInput();
}

void Cube::handleWordsInput() {
	if (IsKeyPressed(KEY_SPACE)) {
		
		if (isSliding || isRolling) { return; }
		
		isPickUp = !isPickUp;
		
		isSwappingWord = true;
		swap.progress = 0.0f;
		
		if (isPickUp) {
			wordCube.pickup();
			PlaySound(pickupWav);
		} else {
			wordCube.release();
			PlaySound(releaseWav);
		}
	}
}

void Cube::handleSlideInput() {
	
	MoveDirection inputDirection = 
		IsKeyPressed(KEY_UP) ? FORWARD_MOVE :
		IsKeyPressed(KEY_DOWN) ? BACKWARD_MOVE :
		IsKeyPressed(KEY_LEFT) ? LEFT_MOVE :
		IsKeyPressed(KEY_RIGHT) ? RIGHT_MOVE :
		NONE;

	if (!isSliding && !isRolling && inputDirection != NONE) {

		if (inputDirection == FORWARD_MOVE) {
			slide.step = {0, 0, -2};
		}
		else if (inputDirection == BACKWARD_MOVE) {
			slide.step = {0, 0, 2};
		}		
		else if (inputDirection == LEFT_MOVE) {
			slide.step = {-2, 0, 0};
		}		
		else if (inputDirection == RIGHT_MOVE) {
			slide.step = {2, 0, 0};
		}
		
		moveDirection = inputDirection;
		slide.start = position;
		endPosition = Vector3Add(position, slide.step);
		
		isSliding = true;
		PlaySound(slideWav);
	}
}

void Cube::updateSliding(float delta) {

	if (isSliding) {
		
		animationProgress += delta * slide.speed;

		// Use smooth easing for animation
		float t = animationProgress;
		float smoothT = t * t * (3.0f - 2.0f * t); // Smoothstep formula
		
		if (smooth) {
			position = Vector3Lerp(slide.start, endPosition, smoothT);
		} else {
			position = Vector3Lerp(slide.start, endPosition, t);
		}

		transformToInitalState();		
		
		if (animationProgress >= 1.0f) {
			position = endPosition;
			cellPos = { (int)position.x/2 + 1, (int)position.z/2 + 1};
			isSliding = false;
			animationProgress = 0.0f;			
		}
	}
}

void Cube::handleRollInput() {
	MoveDirection inputDirection = 
		IsKeyPressed(KEY_W) ? FORWARD_MOVE :
		IsKeyPressed(KEY_A) ? LEFT_MOVE :
		IsKeyPressed(KEY_S) ? BACKWARD_MOVE :
		IsKeyPressed(KEY_D) ? RIGHT_MOVE :
		NONE;

	if (!isRolling && !isSliding && inputDirection != NONE) {

		Vector3 offset; // offset for rotationOrigin
		Vector3 translation; // due to rotation, it gets translated
		
		if (inputDirection == FORWARD_MOVE) {
			offset = {0, -1, -1};
			rollAnim.axisV = X_VECTOR_NEGATIVE;
			rollAnim.axis = X_NEGATIVE;
			translation = {0, 0, -2};
		}
		else if (inputDirection == BACKWARD_MOVE) {
			offset = {0, -1, 1};
			rollAnim.axisV = X_VECTOR_POSITIVE;
			rollAnim.axis = X_POSITIVE;
			translation = {0, 0, 2};
		}		
		else if (inputDirection == LEFT_MOVE) {
			offset = {-1, -1, 0};
			rollAnim.axisV = Z_VECTOR_POSITIVE;
			rollAnim.axis = Z_POSITIVE;
			translation = {-2, 0, 0};
		}		
		else if (inputDirection == RIGHT_MOVE) {
			offset = {1, -1, 0};
			rollAnim.axisV = Z_VECTOR_NEGATIVE;
			rollAnim.axis = Z_NEGATIVE;
			translation = {2, 0, 0};
		}
		
		moveDirection = inputDirection;

		rollAnim.pivot = Vector3Add(position, offset);
		endPosition = Vector3Add(position, translation);
		isRolling = true;
	}
}

void Cube::updateRolling(float delta) {

	if (isRolling) {
		
		animationProgress += delta / rollAnim.duration;
		
		float t = animationProgress;
		float smoothT = t * t * (3.0f - 2.0f * t); // Smoothstep formula

		if (smooth) {
			rollAnim.angle = Lerp(0.0f, 90.0f, smoothT);
		} else {
			rollAnim.angle = Lerp(0.0f, 90.0f, t);
		}
		
		// The cube can have suffered some rotations, so it is likely not to be in the initial rotation state.
		// To proceed with the next rotation, first it is needed to reproduce its rotation state.
		transformToInitalState();
		
		// Now the current rotation is applied using rollAnim.pivot, rollAnim.axisV and the changing rollAnim.angle
		// Combining the matrices: first translate to rollAnim.pivot, then rotating, then translate back
		Matrix translationToPivot = MatrixTranslate(-rollAnim.pivot.x, -rollAnim.pivot.y, -rollAnim.pivot.z);
		Matrix rotation = MatrixRotate(rollAnim.axisV, rollAnim.angle * DEG2RAD);
		Matrix translationBackFromPivot = MatrixTranslate(rollAnim.pivot.x, rollAnim.pivot.y, rollAnim.pivot.z);
		
		Matrix translationPlusRotation = MatrixMultiply(translationToPivot, rotation);
		transforms = MatrixMultiply(transforms, translationPlusRotation);
		transforms = MatrixMultiply(transforms, translationBackFromPivot);
		
		if (animationProgress >= 1.0f) {

			position = endPosition;
			cellPos = { (int)position.x/2 + 1, (int)position.z/2 + 1};

			isRolling = false;
			animationProgress = 0.0f;

			// Add the finished rotation (always 90 degrees) to accumulated rotations
			Matrix finishedRotation = MatrixRotate(rollAnim.axisV, 90.0f * DEG2RAD);
			rotations = MatrixMultiply(rotations, finishedRotation);
			
			transformToInitalState();			
			
			get_next_rotation_state(rotationInfo.state, rollAnim.axis, &rotationInfo.entry);
			strcpy(rotationInfo.state, rotationInfo.entry.state);
			strcpy(rotationInfo.ls_face, rotationInfo.entry.ls_face);
			strcpy(rotationInfo.tg_face, rotationInfo.entry.tg_face);

			// set scale axis for swapping animation
			const char *s = rotationInfo.ls_face;
			if (s[0] == '6') swap.axis = Y_NEGATIVE;
			if (s[0] == '1') swap.axis = Y_POSITIVE;
			if (s[0] == '5') swap.axis = Z_POSITIVE;
			if (s[0] == '3') swap.axis = Z_NEGATIVE;
			if (s[0] == '2') swap.axis = X_POSITIVE;
			if (s[0] == '4') swap.axis = X_NEGATIVE;	
			
			PlaySound(rollWav);
		}
	}
}


void Cube::transformToInitalState() {
	// So, for a cube model translated in current position but with a coordinate system not rotated:
	// 1) it has to be translated to the origin
	// 2) rotated with the stored rotations and 
	// 3) translated back.
	// This is achieved with these matrix multiplications
	Matrix translationToOrigin = MatrixTranslate(-position.x, -position.y, -position.z);
	Matrix translationBackFromOrigin = MatrixTranslate(position.x, position.y, position.z);
	transforms = MatrixMultiply(translationToOrigin, rotations);
	transforms = MatrixMultiply(transforms, translationBackFromOrigin);
}

void Cube::update(float delta) {

	if (isSwappingWord)  {
		swap.progress += delta;
		float t = swap.progress / swap.duration;

		if (t <= 1.0f) {
			// Ease in/out with sine
			float squash = sinf(t * PI); // Goes from 0 to 1 to 0
			swap.currentHeight = swap.baseHeight - squash * (swap.baseHeight - swap.squashHeight);
			swap.scale = 1.0f - squash * (1.0f - swap.squashHeight / swap.baseHeight);
		
		} else {
			isSwappingWord = false;
			swap.currentHeight = swap.baseHeight;
			swap.scale = 1.0f;
		}
	}
	
	updateSliding(delta);
	updateRolling(delta);
}

void drawAxis(Vector3 position, 
			  float axisLength = 5.0f, float axisRadius = 0.02f, 
			  float coneLength = 0.3, float coneRadius = 0.1f);

void Cube::draw() {
	
	Vector3 offset = { 0.0f, 0.0f, 0.0f};
	Vector3 factor = { 1.0f, 1.0f, 1.0f}; // scale factor
	if (isSwappingWord) {
		if (swap.axis == Y_NEGATIVE) {
			offset.y = swap.baseHeight * (1.0f - swap.scale) / 2.0f;
			factor.y = swap.scale;
		}
		if (swap.axis == Y_POSITIVE) {
			offset.y = swap.baseHeight * (swap.scale - 1.0f) / 2.0f;
			factor.y = swap.scale;
		}
		if (swap.axis == Z_NEGATIVE) {
			offset.z = swap.baseHeight * (1.0f - swap.scale) / 2.0f;
			factor.z = swap.scale;
		}
		if (swap.axis == Z_POSITIVE) {
			offset.z = swap.baseHeight * (swap.scale - 1.0f) / 2.0f;
			factor.z = swap.scale;
		}		
		if (swap.axis == X_NEGATIVE) {
			offset.x = swap.baseHeight * (1.0f - swap.scale) / 2.0f;
			factor.x = swap.scale;
		}
		if (swap.axis == X_POSITIVE) {
			offset.x = swap.baseHeight * (swap.scale - 1.0f) / 2.0f;
			factor.x = swap.scale;
		}		

		DrawCubeWires({position.x, position.y + (swap.currentHeight - swap.baseHeight) / 2, position.z}, 
					  2.0, swap.currentHeight, 2.0, PURPLE);
	}

	Matrix scale = MatrixTranslate(offset.x, offset.y, offset.z);
	scale = MatrixMultiply(MatrixScale(factor.x, factor.y, factor.z), scale);
	model.transform = scale;

	rlPushMatrix();
	rlMultMatrixf(MatrixToFloat(transforms));
	drawAxis(position, 1.5f, 0.02, 0.1, 0.05);
	DrawModel(model, position, 1.0f, WHITE);
	rlPopMatrix();
}



// ---------------------------------------------------------------------------------------
// Handle input
// ---------------------------------------------------------------------------------------
bool showFont = false;
bool showWordsTex = true;
bool cameraUpdateEnabled = false;
void handleUiKeys() {
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
}

// ---------------------------------------------------------------------------------------
// Draw Stuff
// ---------------------------------------------------------------------------------------
Camera3D camera;
Camera3D poseCamera;
RenderTexture poseRenderTexture;
Rectangle poseRect = { 0.0f, 0.0f, 300, -168 };

// Draw coordinate axes with cones
void drawAxis(Vector3 p, float axisLength, float axisRadius, float coneLength, float coneRadius) { 
	// X axis (red)
	DrawCylinderEx(p, {p.x+axisLength, p.y, p.z}, axisRadius, axisRadius, 8, RED);
	DrawCylinderEx({p.x+axisLength, p.y, p.z}, 
				   {p.x+axisLength + coneLength, p.y, p.z}, coneRadius, 0.0f, 8, RED);

	// Y axis (green)
	DrawCylinderEx(p, {p.x, p.y+axisLength, p.z}, axisRadius, axisRadius, 8, GREEN);
	DrawCylinderEx({p.x, p.y+axisLength, p.z}, 
				   {p.x, p.y+axisLength + coneLength, p.z}, coneRadius, 0.0f, 8, GREEN);

	// Z axis (blue)
	DrawCylinderEx(p, {p.x, p.y, p.z+axisLength}, axisRadius, axisRadius, 8, BLUE);
	DrawCylinderEx({p.x, p.y, p.z+axisLength}, 
				   {p.x, p.y, p.z+axisLength + coneLength}, coneRadius, 0.0f, 8, BLUE);
}


void DrawScaledText(const char* text, int x, int y, int fontSize, Color color) {
	float scaleX = (float)GetScreenWidth() / SCREEN_WIDTH;
	float scaleY = (float)GetScreenHeight() / SCREEN_HEIGHT;
	float scale = fminf(scaleX, scaleY); // keep aspect ratio

	DrawText(text, x * scale, y * scale, fontSize * scale, color);
}


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
		
		DrawScaledText("Movement data", 10, 130, 20, RED);		
		DrawScaledText(TextFormat("position: (%.2f, %.2f, %.2f)", cube.position.x, cube.position.y, cube.position.z), 
					   10, 150, 20, DARKGRAY);
		DrawScaledText(TextFormat("endPosition: (%.2f, %.2f, %.2f)", 
								  cube.endPosition.x, cube.endPosition.y, cube.endPosition.z), 
					   10, 170, 20, DARKGRAY);
		DrawScaledText(TextFormat("moveDirection: %s", 
								  cube.moveDirection == FORWARD_MOVE ? "forward" :
								  cube.moveDirection == BACKWARD_MOVE ? "backward" :
								  cube.moveDirection == RIGHT_MOVE ? "right" :
								  cube.moveDirection == LEFT_MOVE ? "left" : "idle"),
					   10, 190, 20, DARKGRAY);

		
		DrawScaledText("Arrows slide the cube", 10, 220, 20, RED);
		DrawScaledText(TextFormat("isSliding: %s", cube.isSliding ? "true" : "false"),
					   10, 240, 20, DARKGRAY);
		DrawScaledText(TextFormat("slide.step: (%.1f, %.1f, %.1f)", cube.slide.step.x, cube.slide.step.y, cube.slide.step.z), 
					   10, 260, 20, DARKGRAY);

		
		DrawScaledText("WASD rolls the cube", 10, 300, 20, RED);
		DrawScaledText(TextFormat("isRolling: %s", cube.isRolling ? "true" : "false"),
					   10, 320, 20, DARKGRAY);		
		DrawScaledText(TextFormat("rollAnim.angle: %.2f", cube.rollAnim.angle),
					   10, 340, 20, DARKGRAY);

		
		DrawScaledText("ROTATION", 10, 380, 20, GREEN);
		DrawScaledText(TextFormat("TOP:%s | BOTTOM:%s | state:%s", cube.rotationInfo.ls_face, 
								  cube.rotationInfo.tg_face, cube.rotationInfo.state),
					   10, 400, 20, BLUE);

		DrawScaledText(TextFormat("CellPos:(%i, %i)", cube.cellPos.x, cube.cellPos.z),
					   10, 430, 20, GREEN);
		
		DrawScaledText(TextFormat("animationProgress: %.2f", cube.animationProgress),
					   1000, 680, 20, DARKGRAY);
	}
}

void draw() {
	BeginDrawing();
	float lineWidth = rlGetLineWidth();
	ClearBackground(RAYWHITE);
	BeginMode3D(camera);
	{
		rlSetLineWidth(5.0);
		cube.draw();	
		rlSetLineWidth(lineWidth);
		
		DrawModel(wordCube.model, {1, 0, 1}, 1.0, WHITE);
		
		DrawGrid(10, 2.0f);
		drawAxis({0,0,0});
	}
	EndMode3D();

	
	if (showFont) {
		DrawTexture(fontBubble.texture, SCREEN_WIDTH/2 - fontBubble.texture.width/2, 50, BLUE);
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

// ---------------------------------------------------------------------------------------
// Update stuff
// ---------------------------------------------------------------------------------------
void update(float delta) {

	if (cameraUpdateEnabled) {
		UpdateCamera(&camera, CAMERA_THIRD_PERSON);
	} else {
		cube.update(delta);
	}
}

void updateFrameWindow() {
	
	float delta = GetFrameTime();

	handleUiKeys();
	cube.handleInput();
		
	update(delta);
	
	draw();
}


// ---------------------------------------------------------------------------------------
// Init stuff
// ---------------------------------------------------------------------------------------
void buildTexturePose() {
	BeginTextureMode(poseRenderTexture);
	ClearBackground(RAYWHITE);
	BeginMode3D(poseCamera);
	{
		DrawCube({0,0,0}, 1.0, 1.0, 1.0, SKYBLUE);
		DrawCubeWires({0,0,0}, 1.0, 1.0, 1.0, GREEN);
		drawAxis({0,0,0});
	}
	EndMode3D();
	EndTextureMode();
}

void initWorld() {
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
	// fontKaisg = LoadFontEx("./assets/KAISG.ttf", 64, 0, 0);
	
	wordCube.init("Cube", {25, 45});
}


// ---------------------------------------------------------------------------------------
// Main program
// ---------------------------------------------------------------------------------------
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

int main(void)
{
	SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Cube Of The Words!");
	
	SetTraceLogLevel(LOG_ALL);
	SetTargetFPS(60);
	
	InitAudioDevice();
	
	initWorld();

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
