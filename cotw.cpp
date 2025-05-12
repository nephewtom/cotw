#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "raylib.h"
#include "raymath.h"
#define SUPPORT_TRACELOG
#define SUPPORT_TRACELOG_DEBUG
#include "utils.h"

#include "rlgl.h"

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

struct CellPos {
	int x;
	int z;
	
	bool operator==(const CellPos& other) const {
		if (x == other.x && z == other.z) return true;
		else return false;
	}
};

// ---------------------------------------------------------------------------------------
// Words
// ---------------------------------------------------------------------------------------
Font fontBubble;

void initFont() {
	fontBubble = LoadFontEx("./assets/Game Bubble.ttf", 64, 0, 0);
// fontKaisg = LoadFontEx("./assets/KAISG.ttf", 64, 0, 0);
}

// #define CELL_IMAGE_SIZE 120
// const int CELL_IMAGE_SIZE = 120; // in pixels
// #define FONT_SIZE fontBubble.baseSize/2;
const int CELL_IMAGE_SIZE = 480; // in pixels
#define FONT_SIZE (float)4*fontBubble.baseSize/2

struct Word {
	char* text;
	
	Model model;

	Texture2D texture;
	Vector2 offset;
	Image wordImage;
	Image blankImage;
	
	CellPos cellPos;
	bool onCube;
	
	void init(const char* word) {
		text = strdup(word);
		TRACELOGD("Word:%s, ", text);
		blankImage = GenImageColor(CELL_IMAGE_SIZE, CELL_IMAGE_SIZE, BLANK);
		
		Mesh planeMesh = GenMeshPlane(2.0, 2.0, 1, 1);
		model = LoadModelFromMesh(planeMesh);
		
		onCube = false;
	}
	
	// offset - 2D offset in the 2D texture from top-left corner	
	void applyTexture() {
		wordImage = GenImageColor(CELL_IMAGE_SIZE, CELL_IMAGE_SIZE, BLANK);
		// RLAPI void ImageDrawTextEx(Image *dst, Font font, const char *text, 
		//                            Vector2 position, float fontSize, float spacing, Color tint);
		ImageDrawTextEx(&wordImage, fontBubble, text, offset, FONT_SIZE, 0.0f, BLACK);
		texture = LoadTextureFromImage(wordImage);
		
		for (int i=0; i<model.materialCount; i++) {
			model.materials[i].maps[MATERIAL_MAP_DIFFUSE].texture = texture;
			model.materials[i].maps[MATERIAL_MAP_ALBEDO].texture = texture;
		}
	}

	void pickup() {
		// Pick up the word, so it disappears from the ground
		UpdateTexture(texture, blankImage.data);
		onCube = true;
	}
	
	void release(CellPos cubeCellPos) {
		// Release the word, so it appears on the ground
		UpdateTexture(texture, wordImage.data);
		onCube = false;
		cellPos = cubeCellPos;
	}
	
	void draw() {
		// cell positions are multiplied per 2 to match space coordinates
		if (!onCube) {
			DrawModel(model, {(float)2*cellPos.x, 0.1, (float)2*cellPos.z}, 1.0, WHITE);
		}
	}
	
	void unload() {
		free(text);
		UnloadModel(model);
		UnloadTexture(texture);
		UnloadImage(wordImage);
		UnloadImage(blankImage);
	}
};

Word wordCube;

int countWords(const char *text) {
    int count = 0;
    bool inWord = false;

	for (const char *c = text; *c; c++) {
		if (*c != ' ' && *c != '\t' && *c != '\n') {
			if (!inWord) {
				count++;
				inWord = true;
			}
        } else {
			inWord = false;
		}
    }

    return count;
}


// ---------------------------------------------------------------------------------------
// Quotes
// ---------------------------------------------------------------------------------------
#include <vector>

struct QuoteData {
	const char* quote;
	int wordCount;
	std::vector<Vector2> offset;
	std::vector<CellPos> cellPos;
};

enum QuoteAuthor{
	NIKE = 0, OPUS, STEVE_JOBS, PABLO_PICASSO
};

std::vector<QuoteData> quotesData = {
    {
        "Just do it!", 4, 
        { {0.2, 0.375}, {0.2, 0.375}, {0.2, 0.375} }, 
        { {0,0}, {0,2}, {-1,1} }
    },
    {
        "Life is life.", 3, 
		{ {0.2, 0.375}, {0.2, 0.375}, {0.2, 0.375} }, 
        { {1,0}, {1,1}, {1,2} }
    },
    {
        "Stay hungry, stay foolish.", 4, 
		{ {0.2, 0.375}, {0.2, 0.375}, {0.2, 0.375}, {0.2, 0.375} }, 
        { {2,0}, {2,1}, {2,2}, {2,3} }
    },
	{
        "Everything you can imagine is real.", 6, 
		{ {0.2, 0.375}, {0.2, 0.375}, {0.2, 0.375}, {0.2, 0.375}, {0.2, 0.375}, {0.2, 0.375} },
		{ {2,0}, {2,1}, {2,2}, {2,3}, {2,3}, {2,3} }
    }
	
};

struct Quote {
	Word* words;
	int wordCount;

	void create(QuoteData& data) {
		
		wordCount = countWords(data.quote);
		printf("wordCount: %i\n", wordCount);

		if (wordCount != data.wordCount) {
			TRACELOG(LOG_ERROR, "Mismatch in wordCount!");
			TRACELOG(LOG_ERROR, "quote: %s", data.quote);
		}

		initWords(data.quote);
		for (int i=0; i<wordCount; i++) {
			words[i].offset = Vector2Scale(data.offset[i], CELL_IMAGE_SIZE);
			words[i].applyTexture();
			words[i].cellPos = data.cellPos[i];
		}
	}

	void draw() {
		for (int i=0; i<wordCount; i++) {
			words[i].draw();			
		}
	}
	
	void unload() {
		for (int i=0; i<wordCount; i++) {
			words[i].unload();
		}
	}
	
	void initWords(const char *text) {
	
		words = (Word*) malloc(sizeof(Word) * wordCount);
		if (!words) { 
			fprintf(stderr, "Words allocation failed!\n");
			exit(-1);
		}
	
		char* buffer = strdup(text);
		if (!buffer) {
			fprintf(stderr, "strdup failed!\n");
			free(words);
			exit(-1);
		}
	
		int index = 0;
		char *token = strtok(buffer, " \n\t");
		while (token && index < wordCount) {
		
			Word *w = &words[index];
			w->init(token);

			index++;
			token = strtok(NULL, " \n\t");
		}
	}
};

Quote quote;


// ---------------------------------------------------------------------------------------
// Cube stuff
// ---------------------------------------------------------------------------------------
#include "rt.h"

struct RotationInfo {
	char state[32];
	char TG_face[16]; // Touching Ground face
	char LS_face[16]; // Looking Sky face
	RotationEntry entry;	
};

enum MoveDirection {
	NONE, FORWARD_MOVE, BACKWARD_MOVE, RIGHT_MOVE, LEFT_MOVE
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

	bool isPicking = false;
	struct PickingAnim {
		float progress = 0.0f;
		float duration = 0.3f;
		float baseHeight = 2.0f;
		float currentHeight = baseHeight;
		float squashHeight = 1.2f;
		float scale = 1.0f;
		Axis axis;
	};
	PickingAnim picking;
	void setScaleAxis();
	
	CellPos cellPos;
	int wordIndexOnGround; // wordIndex in quote or -1 if there is no word under cube
	int updateWordIndexOnGround(); // updates the word index in quote or -1 if not found
	bool isWordOnGround();
	
	int	wordIndexAtFace[7]; // 0 element is not used, so the index matches with faces 1-6
	bool isWordOnTouchingGroundFace();
	
	
	Sound rollWav;
	Sound slideWav;
	Sound pickupWav;
	Sound releaseWav;
	Sound errorWav;
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
	cellPos = { (int)position.x/2, (int)position.z/2 };

	transforms = MatrixIdentity();
	
	rotations = MatrixIdentity(); // cube has no rotations at start
	
	smooth = smoothBehaviour;
	
	rollWav = LoadSound("assets/roll.wav");
	slideWav = LoadSound("assets/slide.wav");
	pickupWav = LoadSound("assets/pickup.wav");
	releaseWav = LoadSound("assets/release.wav");
	errorWav = LoadSound("assets/error.wav");
	
	// Check rt.h to understand this
	init_state_map();
	strcpy(rotationInfo.state, "i");
	strcpy(rotationInfo.LS_face, "1U");
	strcpy(rotationInfo.TG_face, "6D");	
	picking.axis = Y_POSITIVE;
	
	for (int i=0; i<7; i++) {
		wordIndexAtFace[i] = -1;
	}
	wordIndexOnGround = -1;
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
		
		isPicking = true;
		
		if (isWordOnGround() && isWordOnTouchingGroundFace()) {
			// there is a word on the ground AND the cube has a word in its Touching Ground face
			PlaySound(errorWav);
		} else if (!isWordOnGround() && !isWordOnTouchingGroundFace()) {
			// there is no word on the ground AND the cube has not word in its TG face
			PlaySound(errorWav);
		} else if (isWordOnGround()) {
			// the cube picks up the word
			int groundFaceIndex = rotationInfo.TG_face[0] - '0';
			wordIndexAtFace[groundFaceIndex] = wordIndexOnGround;
			
			Word& word = quote.words[wordIndexOnGround];
			wordIndexOnGround = -1;

			word.pickup();
			PlaySound(pickupWav);
			
		} else if (isWordOnTouchingGroundFace()) {
			// the cube releases the word on the ground
			int groundFaceIndex = rotationInfo.TG_face[0] - '0';
			wordIndexOnGround = wordIndexAtFace[groundFaceIndex];
			wordIndexAtFace[groundFaceIndex] = -1;
			
			Word& word = quote.words[wordIndexOnGround];
			word.release(cellPos);
			PlaySound(releaseWav);
		}
		
		picking.progress = 0.0f;
	}
}

bool Cube::isWordOnTouchingGroundFace() {
	int groundFaceIndex = rotationInfo.TG_face[0] - '0';
	return wordIndexAtFace[groundFaceIndex]  != -1;
}

bool Cube::isWordOnGround() {
	return	wordIndexOnGround != -1 ;
} 

int Cube::updateWordIndexOnGround() {
	
	for (int i=0; i<quote.wordCount; i++) {
		Word& word = quote.words[i];
		if (!word.onCube && cellPos == word.cellPos) {
			return i;
		}
	}
	return -1;
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
			cellPos = { (int)position.x/2, (int)position.z/2 };
			isSliding = false;
			animationProgress = 0.0f;
			
			wordIndexOnGround = updateWordIndexOnGround();
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
				cellPos = { (int)position.x/2, (int)position.z/2};

				isRolling = false;
				animationProgress = 0.0f;

				// Add the finished rotation (always 90 degrees) to accumulated rotations
				Matrix finishedRotation = MatrixRotate(rollAnim.axisV, 90.0f * DEG2RAD);
				rotations = MatrixMultiply(rotations, finishedRotation);
			
				transformToInitalState();			
			
				get_next_rotation_state(rotationInfo.state, rollAnim.axis, &rotationInfo.entry);
				strcpy(rotationInfo.state, rotationInfo.entry.state);
				strcpy(rotationInfo.LS_face, rotationInfo.entry.LS_face);
				strcpy(rotationInfo.TG_face, rotationInfo.entry.TG_face);

				// set scale axis for swapping animation
				const char *s = rotationInfo.LS_face;
				if (s[0] == '6') picking.axis = Y_NEGATIVE;
				if (s[0] == '1') picking.axis = Y_POSITIVE;
				if (s[0] == '5') picking.axis = Z_POSITIVE;
				if (s[0] == '3') picking.axis = Z_NEGATIVE;
				if (s[0] == '2') picking.axis = X_POSITIVE;
				if (s[0] == '4') picking.axis = X_NEGATIVE;	

			
				wordIndexOnGround = updateWordIndexOnGround();
			
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

		if (isPicking)  {
			picking.progress += delta;
			float t = picking.progress / picking.duration;

			if (t <= 1.0f) {
				// Ease in/out with sine
				float squash = sinf(t * PI); // Goes from 0 to 1 to 0
				picking.currentHeight = picking.baseHeight - squash * (picking.baseHeight - picking.squashHeight);
				picking.scale = 1.0f - squash * (1.0f - picking.squashHeight / picking.baseHeight);
		
			} else {
				isPicking = false;
				picking.currentHeight = picking.baseHeight;
				picking.scale = 1.0f;
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
		if (isPicking) {
			if (picking.axis == Y_NEGATIVE) {
				offset.y = picking.baseHeight * (1.0f - picking.scale) / 2.0f;
				factor.y = picking.scale;
			}
			if (picking.axis == Y_POSITIVE) {
				offset.y = picking.baseHeight * (picking.scale - 1.0f) / 2.0f;
				factor.y = picking.scale;
			}
			if (picking.axis == Z_NEGATIVE) {
				offset.z = picking.baseHeight * (1.0f - picking.scale) / 2.0f;
				factor.z = picking.scale;
			}
			if (picking.axis == Z_POSITIVE) {
				offset.z = picking.baseHeight * (picking.scale - 1.0f) / 2.0f;
				factor.z = picking.scale;
			}		
			if (picking.axis == X_NEGATIVE) {
				offset.x = picking.baseHeight * (1.0f - picking.scale) / 2.0f;
				factor.x = picking.scale;
			}
			if (picking.axis == X_POSITIVE) {
				offset.x = picking.baseHeight * (picking.scale - 1.0f) / 2.0f;
				factor.x = picking.scale;
			}		

			DrawCubeWires({position.x, position.y + (picking.currentHeight - picking.baseHeight) / 2, position.z}, 
						  2.0, picking.currentHeight, 2.0, PURPLE);
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


	void drawGrid(int slices, float spacing)
	{
		int halfSlices = slices/2;

		rlBegin(RL_LINES);
		for (int i = -halfSlices; i <= halfSlices; i++)
		{
			if (i == 0)
			{
				rlColor3f(0.5f, 0.5f, 0.5f);
			}
			else
			{
				rlColor3f(0.75f, 0.75f, 0.75f);
			}

			rlVertex3f((float)i*spacing+1, 0.0f, (float)-halfSlices*spacing);
			rlVertex3f((float)i*spacing+1, 0.0f, (float)halfSlices*spacing);

			rlVertex3f((float)-halfSlices*spacing, 0.0f, (float)i*spacing+1);
			rlVertex3f((float)halfSlices*spacing, 0.0f, (float)i*spacing+1);
		}
rlEnd();
}



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


void drawTextControls() {
	int px = 10; int py = 30; // start y position
	
	DrawText(TextFormat("Q - toggle camera control: %s", cameraUpdateEnabled ? "ON" : "OFF"),
			 px, py, 20, BLUE);
	DrawText(TextFormat("WASD & Arrows - control %s", cameraUpdateEnabled ? "camera" : "cube"),
			 px, py+20, 20, BLUE);
	DrawText(TextFormat("E - toggle cube numbers/words texture"),
			 px, py+50, 20, BLUE);
	DrawText(TextFormat("T - toggle hide 2D texture"),
			 px, py+70, 20, BLUE);
}

void drawTextMovement() {
	int px = 10; int py = 160;
	DrawText("Movement data", px, py, 20, RED);		
	DrawText(TextFormat("position: (%.2f, %.2f, %.2f)", 
						cube.position.x, cube.position.y, cube.position.z), 
			 px, py+20, 20, DARKGRAY);
	DrawText(TextFormat("endPosition: (%.2f, %.2f, %.2f)", 
						cube.endPosition.x, cube.endPosition.y, cube.endPosition.z), 
			 px, py+40, 20, DARKGRAY);
	DrawText(TextFormat("moveDirection: %s", 
						cube.moveDirection == FORWARD_MOVE ? "forward" :
						cube.moveDirection == BACKWARD_MOVE ? "backward" :
						cube.moveDirection == RIGHT_MOVE ? "right" :
						cube.moveDirection == LEFT_MOVE ? "left" : "idle"),
			 px, py+60, 20, DARKGRAY);
}

void drawTextAnimation() {
	int px = 10; int py = 260;
	DrawText("Arrows slide the cube", px, py, 20, RED);
	DrawText(TextFormat("isSliding: %s", cube.isSliding ? "true" : "false"),
			 px, py+20, 20, DARKGRAY);
	DrawText(TextFormat("slide.step: (%.1f, %.1f, %.1f)", 
						cube.slide.step.x, cube.slide.step.y, cube.slide.step.z), 
			 px, py+40, 20, DARKGRAY);

	py = 340;
	DrawText("WASD rolls the cube", px, py, 20, RED);
	DrawText(TextFormat("isRolling: %s", cube.isRolling ? "true" : "false"),
			 px, py+20, 20, DARKGRAY);		
	DrawText(TextFormat("rollAnim.angle: %.2f", cube.rollAnim.angle),
			 px, py+40, 20, DARKGRAY);
		
	py = 420;
	DrawText(TextFormat("animationProgress: %.2f", cube.animationProgress),
			 px, py, 20, DARKGRAY);
}

void drawTextPickup() {
	int px = 900; int py = 10; 
	DrawText("ROTATION", px, py, 20, GREEN);
	DrawText(TextFormat("TOP:%s | BOTTOM:%s | state:%s", cube.rotationInfo.LS_face, 
						cube.rotationInfo.TG_face, cube.rotationInfo.state),
			 px, py+20, 20, BLUE);

	DrawText("CellPos:", px, py+50, 20, GREEN);
	DrawText(TextFormat("(%i, %i)", cube.cellPos.x, cube.cellPos.z),
			 px+100, py+50, 20, BLUE);
	

	py = 150;
	DrawText(TextFormat("isWordOnGround: %s", cube.isWordOnGround() ? "true" : "false"),
			 px, py, 20, BLUE);
	DrawText(TextFormat("isWordOnTouchingGroundFace: %s", cube.isWordOnTouchingGroundFace() ? "true" : "false"),
			 px, py+20, 20, BLUE);
	
	py = 200;
	DrawText("wordIndexAtFace", px, py, 20, GREEN);
	for (int i=1; i<7; i++) {
		DrawText(TextFormat("Face %i: %i", i, cube.wordIndexAtFace[i]), px, py+i*20, 20, BLUE);
	}
	
}

void drawTextQuote() {
	int px = 700; int py = 600;
	DrawText("QUOTE", px, py, 20, GREEN);
	for (int i=0; i<quote.wordCount; i++) {
		Word& word = quote.words[i];
		DrawText(TextFormat("word: %s | cellPos:(%i, %i) | onCube:%s", word.text, word.cellPos.x, word.cellPos.z, 
							word.onCube ? "true" : "false"), px, py+(i+1)*20, 20, BLUE);
	}	
}

void drawText() {
	DrawFPS(10, 10);
	drawTextControls();
	
	if (!cameraUpdateEnabled) {
		drawTextMovement();
		drawTextAnimation();
		drawTextPickup();
		drawTextQuote();
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
		
		wordCube.draw();
		quote.draw();
		
		drawGrid(10, 2.0f);
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
	// DrawTextureRec(poseRenderTexture.texture, poseRect, {960.0f, 20.0f}, WHITE);
	// DrawRectangleLines(960, 20, 300, 168, GREEN);
	
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
	camera.position = { 3.0f, 10.0f, 12.0f };
	camera.target = { 0.0f, 0.0f, 0.0f };
	camera.up = { 0.0f, 1.0f, 0.0f };
	camera.fovy = 45.0f;
	camera.projection = CAMERA_PERSPECTIVE;

	cube.init({4, 1, 0}, true);
	
	poseCamera.position = { 1.5f, 1.5f, 1.5f };
	poseCamera.target = { 0.0f, 0.0f, 0.0f };
	poseCamera.up = { 0.0f, 1.0f, 0.0f };
	poseCamera.fovy = 45.0f;
	poseCamera.projection = CAMERA_PERSPECTIVE;
	poseRenderTexture = LoadRenderTexture(300, 168);
	buildTexturePose();
	
	initFont();
	
	wordCube.init("Cube");
	wordCube.offset = {0.2*CELL_IMAGE_SIZE, 0.375*CELL_IMAGE_SIZE};
	wordCube.applyTexture();
	wordCube.cellPos = { -2, -2 };
	
	quote.create(quotesData[NIKE]);
	   
	fflush(stdout);
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
