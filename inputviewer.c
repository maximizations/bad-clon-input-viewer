// TODO : make it not suck
// im making a bit of progress -- its now like acceptable lmao


#include <stdint.h>
#include "stivector.h"
#include "raylib.h"

// constants
const int DO_LIMIT_FPS = 1;
const int MAX_FPS = 1000;
const bool SHOW_FPS_COUNT = 0;

const int INIT_WINDOW_WIDTH = 600;
const int INIT_WINDOW_HEIGHT = 75;

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

const Color BACKGROUND_COLOR = BLACK;

const int BUTTON_COUNT = 7; // set to 8 for 6 fret
const float BUTTON_BORDER_SIZE = 3.0f;

const float BAR_SPEED = 400.0f;

#define BUTTON_WIDTH (float)GetScreenWidth() / BUTTON_COUNT - BUTTON_BORDER_SIZE
#define BUTTON_HEIGHT BUTTON_WIDTH

// keybinds
#define ID_KEY_DOWN (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S) || IsKeyPressed(KEY_J))
#define ID_KEY_UP (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W) || IsKeyPressed(KEY_K))
#define ID_KEY_REFRESH ((IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) && IsKeyPressed(KEY_R)) || IsKeyPressed(KEY_F5)
#define ID_KEY_ENTER (IsKeyPressed(KEY_ENTER)) 
#define ID_KEY_BREAK (IsKeyPressed(KEY_ESCAPE))


// simple macros for simple things
#define clamp(f, min, max) ((f) < (min) ? (min) : (f) > (max) ? (max) : (f))
#define max(x, y) ((x) > (y) ? (x) : (y))
#define min(x, y) ((x) < (y) ? (x) : (y))


// """""scenes""""" (there are only 2 lmao)
// TODO : turn this into IDSceneID and make a struct with needsScreenWidth and needsScreenHeight so i can compute them only when needed
//        without doing it in my update functions
typedef enum {
    INPUT_VIEWER,
    CONTROLLER_SELECTOR
} IDScene;


// buttons
typedef struct {
    Rectangle rectangle;
    float border;
    int key;
    unsigned int inputCount;
    double initTime, updateTime;
    Color color;
    bool pressed;

    stivector idbarVector;
} IDButton;

// bars
typedef struct {
    Rectangle rectangle;
    Color color;
    bool is_held;
} IDBar;

// gamepad state
typedef struct {
    int gamepadCount;
    int selectedGamepad;
    bool gamepadNeedsRefresh;
} IDGamepadState;

// button creation functions
IDButton createButton (Rectangle rec, float border, Color color) {
    IDButton button;
        button.rectangle = rec;
        button.border = border;
        button.inputCount = 0;
        button.initTime = GetTime();
        button.updateTime = GetTime();
        button.color = color;
        button.key = 0;
        button.pressed = 0;

        //stivector vector;
        //button.idbarVector = vector;
        stivector_init(&button.idbarVector, 10, sizeof(IDBar));

   return button;
}

IDButton* createButtonArray(size_t arrayLength, float buttonWidth, float buttonHeight, float borderSize) {  
    IDButton *buttons = (IDButton*)malloc(arrayLength * sizeof(IDButton));
    
    for (size_t i = 0; i < arrayLength; i++) {
        buttons[i] = createButton((Rectangle){ (float)i * (buttonWidth + borderSize), 0.0f, buttonWidth, buttonHeight }, borderSize, GREEN);
    }
    
    return buttons;
}

void setButtonColor5Fret(IDButton *buttons) {
   buttons[0].color = GREEN;
   buttons[1].color = RED;
   buttons[2].color = YELLOW;
   buttons[3].color = BLUE;
   buttons[4].color = ORANGE;
   buttons[5].color = LIGHTGRAY;
   buttons[6].color = LIGHTGRAY;
}

void setButtonColor6Fret(IDButton *buttons) {
   buttons[0].color = WHITE;
   buttons[1].color = WHITE;
   buttons[2].color = WHITE;
   buttons[3].color = DARKGRAY;
   buttons[4].color = DARKGRAY;
   buttons[5].color = DARKGRAY;
   buttons[6].color = LIGHTGRAY;
   buttons[7].color = LIGHTGRAY;
}

// initialize our gamepad state
void initGamepadState(IDGamepadState *gamepadState) {
    gamepadState->gamepadCount = 0;
    gamepadState->selectedGamepad = 0;
    gamepadState->gamepadNeedsRefresh = 1;
}

// bar update functions
IDBar createBar (IDButton *button) {
    IDBar bar;
        bar.rectangle = (Rectangle){ button->rectangle.x + button->border,
                                     button->rectangle.y + button->rectangle.height,
                                     button->rectangle.width - button->border - button->border,
                                     BAR_SPEED * GetFrameTime() };
        bar.color = button->color;
        bar.is_held = 1;

    return bar;
}

void freeBarVecs(IDButton *buttons, size_t arrayLength)  {
    for (size_t i = 0; i < arrayLength; i++) {
        stivector_free(&(buttons[i].idbarVector));
    }
}

void cleanBarVec (IDButton *button) {
    if (button->idbarVector.size <= 0) {
        return;
    }

    IDBar *bar = (IDBar*)stivector_at(&(button->idbarVector), 0);

    if (bar->rectangle.y > (float)GetScreenHeight()) {
        stivector_erase(&(button->idbarVector), 0);
    }
}


void updateBar (IDBar *bar) {
    if (bar->is_held) {
        bar->rectangle.height += BAR_SPEED * GetFrameTime();
    } else {
        bar->rectangle.y += BAR_SPEED * GetFrameTime();
    }
}

void updateBarVec (IDButton *button) {
    if (button->idbarVector.size <= 0) {
        return;
    }

    for (size_t i = 0; i < button->idbarVector.size; i++) {
        IDBar *currentBar = (IDBar*)stivector_at(&(button->idbarVector), i);

        updateBar(currentBar);
    }
}

void releaseBar (IDButton *button) { 
    if (button->idbarVector.size <= 0) {
        return;
    }

    IDBar *heldBar = (IDBar*)stivector_at(&(button->idbarVector), button->idbarVector.size - 1);

    heldBar->is_held = 0;
}


// button update functions
void buttonInputReleased (IDButton *button, IDGamepadState *gamepadState) {
    if (IsGamepadButtonReleased(gamepadState->selectedGamepad, button->key)) {
        releaseBar(button); 
    }
}

void updateTimer (IDButton *button) {
    button->updateTime = GetTime();
}

bool isButtonClicked(IDButton *button) {
    Vector2 mousePos = GetMousePosition();
    
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        return CheckCollisionPointRec(mousePos, button->rectangle);
    }
    
    return 0;
}

void setButtonPressed(IDButton *button, IDGamepadState *gamepadState) {
    if (IsGamepadButtonDown(gamepadState->selectedGamepad, button->key)) {
        button->pressed = 1;
        updateTimer(button);
    } else {
        button->pressed = 0;
    }
}

void buttonInputPressed(IDButton *button, IDGamepadState *gamepadState) {
    if (IsGamepadButtonPressed(gamepadState->selectedGamepad, button->key)) {
        button->inputCount++;
        button->initTime = GetTime();
        IDBar newBar = createBar(button);

        stivector_push_back(&(button->idbarVector), &newBar);
    }
}

void updateButtonSize(IDButton *button, int index) {
    button->rectangle.width = BUTTON_WIDTH;
    button->rectangle.height = BUTTON_HEIGHT;
    button->rectangle.x = (float)index * (button->rectangle.width + button->border);
}

void updateClickedButton(IDButton *button, int *clickedButton) {
    int key = GetGamepadButtonPressed();
    
    if (key != 0) {
        button->key = key;
        *clickedButton = -1;
    }
}

// this one is on its own
void setGamepadCount(IDGamepadState *gamepadState) {
    while (IsGamepadAvailable(gamepadState->gamepadCount)) {
        (gamepadState->gamepadCount)++;
    }
    while (!IsGamepadAvailable(gamepadState->gamepadCount) && gamepadState->gamepadCount > 0) {
        (gamepadState->gamepadCount)--;
        while (gamepadState->selectedGamepad > gamepadState->gamepadCount - 1) {
            (gamepadState->selectedGamepad)--;
        }
    }
}

void manageButtonClicked(IDButton *buttons, int *clickedButton) {
        IDButton *currentClickedButton = &buttons[*clickedButton];

        releaseBar(currentClickedButton);
        updateClickedButton(currentClickedButton, clickedButton);
}

void manageButtonInput(IDButton *button, IDGamepadState *gamepadState) {
        setButtonPressed(button, gamepadState);
        buttonInputPressed(button, gamepadState);
        buttonInputReleased(button, gamepadState);
}

// big button update function
void updateButton(IDButton *buttons, size_t buttonIndex, int *screenWidth, bool *screenWidthChanged, int *clickedButton, IDGamepadState *gamepadState) {
    IDButton *currentButton = &buttons[buttonIndex];
    
    // update the size of the buttons if the screen width changes
    if (*screenWidth != GetScreenWidth()) {
        updateButtonSize(currentButton, buttonIndex);
        *screenWidthChanged = 1;
    }

    // for when a button is left clicked, so you can set a keybind
    if (isButtonClicked(currentButton) && *clickedButton < 0) {
        *clickedButton = buttonIndex;
        currentButton->pressed = 1;
    }

    if (*clickedButton < 0) {
        manageButtonInput(currentButton, gamepadState);
    } else {
        manageButtonClicked(buttons, clickedButton);
    }

    // deal with the idbars
    cleanBarVec(currentButton);
    updateBarVec(currentButton);
}

void updateButtons(IDButton *buttons, size_t arrayLength, int *screenWidth, int *clickedButton, IDGamepadState *gamepadState, IDScene *currentScene) {
    bool screenWidthChanged = 0;
        
    for (size_t i = 0; i < arrayLength; i++) {
        updateButton(buttons, i, screenWidth, &screenWidthChanged, clickedButton, gamepadState);
    }

    // updates the screen width -- could belong somewhere else -- TODO
    if (screenWidthChanged) {
        *screenWidth = GetScreenWidth();
    }
    
    if (ID_KEY_BREAK) {
        *clickedButton = -1;
    }

    if (ID_KEY_ENTER && *clickedButton < 0) {
        *currentScene = CONTROLLER_SELECTOR;
        gamepadState->gamepadNeedsRefresh = 1;
    }
}


// big controller switcher ui update function
void updateControllerSwitcherUI(IDGamepadState *gamepadState, IDScene *currentScene) {
    if (ID_KEY_DOWN && gamepadState->selectedGamepad < gamepadState->gamepadCount - 1) {
        (gamepadState->selectedGamepad)++;
        while (!IsGamepadAvailable(gamepadState->selectedGamepad)) {
            (gamepadState->selectedGamepad)++;
            if (gamepadState->selectedGamepad >= gamepadState->gamepadCount - 1) {
                break;
            }
        }
    }

    if (ID_KEY_UP && gamepadState->selectedGamepad > 0) {
        (gamepadState->selectedGamepad)--;
        while (!IsGamepadAvailable(gamepadState->selectedGamepad)) {
            (gamepadState->selectedGamepad)--;
            if (gamepadState->selectedGamepad <= 0) {
                break;
            }
        }
    }
    
    if (ID_KEY_REFRESH) {
        gamepadState->gamepadNeedsRefresh = 1;
    }

    if (ID_KEY_ENTER) {
        *currentScene = INPUT_VIEWER;
    }
}


// DRAWING FUNCTIONS



// button drawing functions
float fitStringInButton(IDButton *button, Font font, const char *string, float fontSize, float fontSpacing, Vector2 textSize) {
    float rightFontSize = fontSize;
    Vector2 updateTextSize = textSize; 

    while (updateTextSize.x > button->rectangle.width - button->border) {
        rightFontSize--;
        updateTextSize = MeasureTextEx(font, string, rightFontSize, fontSpacing);
    }

    return rightFontSize;
}

Vector2 getCountStringPosition(IDButton *button, Vector2 textSize) {
    float stringX = button->rectangle.x + button->rectangle.width * 0.5f - textSize.x * 0.5f;
    float stringY = button->rectangle.y + button->rectangle.height * 0.5f - textSize.y * 0.5f;
    
    Vector2 stringPosition = (Vector2){ stringX, stringY };

    return stringPosition;
} 

Vector2 getTimerStringPosition(IDButton *button, Vector2 textSize) {
    float stringX = button->rectangle.x + button->rectangle.width * 0.5f - textSize.x * 0.5f;
    float stringY = button->rectangle.y + button->rectangle.height - textSize.y - textSize.y;
    
    Vector2 stringPosition = (Vector2){ stringX, stringY };

    return stringPosition;
} 

void drawInputCount(IDButton *button, Font font, float fontSize, float fontSpacing) {
    char inputcount[12];
    snprintf(inputcount, 12, "%d", button->inputCount);
    
    Vector2 textSize = MeasureTextEx(font, inputcount, fontSize, fontSpacing);

    float rightFontSize = fitStringInButton(button, font, inputcount, fontSize, fontSpacing, textSize);
    Vector2 stringPosition = getCountStringPosition(button, textSize);
    
    DrawTextEx(font, inputcount, stringPosition, rightFontSize, fontSpacing, WHITE);
}

void drawInputTimer(IDButton *button, Font font, float fontSize, float fontSpacing) {
    char inputtimer[16];
    snprintf(inputtimer, 16, "%.3f", button->updateTime - button->initTime);
    
    Vector2 textSize = MeasureTextEx(font, inputtimer, fontSize, fontSpacing);

    float rightFontSize = fitStringInButton(button, font, inputtimer, fontSize, fontSpacing, textSize);
    Vector2 stringPosition = getTimerStringPosition(button, textSize);

    DrawTextEx(font, inputtimer, stringPosition, rightFontSize, fontSpacing, WHITE);
}

void drawBar(IDButton *button) {
    for (size_t i = 0; i < button->idbarVector.size; i++) {
        IDBar *currentBar = (IDBar*)stivector_at(&(button->idbarVector), i);

        DrawRectangleRec(currentBar->rectangle, currentBar->color);
    }
}

void drawButtons(IDButton *buttons, size_t arrayLength) {
    for (size_t i = 0; i < arrayLength; i++) {
        IDButton currentButton = buttons[i];

        if (currentButton.pressed) {
            DrawRectangleRec(currentButton.rectangle, currentButton.color);
        } else {
            DrawRectangleLinesEx(currentButton.rectangle, currentButton.border, currentButton.color);
        }

        drawInputCount(&currentButton, GetFontDefault(), currentButton.rectangle.width * 0.4f, 2.0f);
        drawInputTimer(&currentButton, GetFontDefault(), currentButton.rectangle.width * 0.16f, 2.0f);
        drawBar(&currentButton);
    }
}


// controller selector drawing functions
void drawControllerSelectorTriangle(float height, Vector2 textPosition, Color textColor) {
    Vector2 point1 = (Vector2){ textPosition.x - height * 1.5f, textPosition.y };
    Vector2 point2 = (Vector2){ point1.x + height, point1.y + 0.5f * height };
    Vector2 point3 = (Vector2){ point1.x, point1.y + height };

    DrawTriangle(point3, point2, point1, textColor);

    Vector2 shadowPoint2 = (Vector2){ point1.x + height * 0.5f, point1.y + 0.5f * height };
    Vector2 shadowPoint3 = (Vector2){ point1.x, point1.y + height };

    DrawTriangle(shadowPoint3, shadowPoint2, point1, BACKGROUND_COLOR); 
}

void drawControllerSelectorTitle(Font font, float fontSize, float fontSpacing) {
    Vector2 textPosition = (Vector2){ 0.0f, fontSize * 0.5f };

    DrawTextEx(font, "Controller selector", textPosition,  fontSize, fontSpacing, WHITE);
}

void drawControllerSelectorEntry(int index, int pos, Font font, const char *gamepadName, float fontSize, float fontSpacing, IDGamepadState *gamepadState, int screenWidth, int screenHeight) {
    Vector2 textPosition = (Vector2){ screenWidth * 0.125f, pos * fontSize + screenHeight * 0.25f };
    
    Color textColor = WHITE; 
    
    if (index == gamepadState->selectedGamepad) {
        textColor = BLUE;
        drawControllerSelectorTriangle(fontSize, textPosition, textColor);
    }

    DrawTextEx(font, gamepadName, textPosition, fontSize, fontSpacing, textColor); 
}

// TODO: THIS FUNCTION IS ASS AND HAS BASICALLY NO THOUGHT PUT INTO IT, 2 BE REWRITTEN LOL THERE IS NO LOGIC BEHIND ANYTHING 
void drawControllerSwitcherUI(IDGamepadState *gamepadState) {
    //TODO : more global management of this, less confusing mess
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    float titleFontSize = clamp(min((float)screenWidth, (float)screenHeight) * 0.1f, 24.0f, 40.0f);
    drawControllerSelectorTitle(GetFontDefault(), titleFontSize , 2.0f);

    for (int i = 0, pos = 0; i < gamepadState->gamepadCount; i++, pos++) {
        const char *gamepadName = GetGamepadName(i);

        if (*gamepadName != '\0') {
            float entryFontSize = clamp(min((float)screenWidth, (float)screenHeight) * 0.05f, 16.0f, 24.0f);

            drawControllerSelectorEntry(i, pos, GetFontDefault(), gamepadName, entryFontSize, 2.0f, gamepadState, screenWidth, screenHeight);
        } else {
            pos--;
        }
    }
}


// main
int main () { 
    InitWindow(INIT_WINDOW_WIDTH, INIT_WINDOW_HEIGHT, "bad ch input viewer");

    if (DO_LIMIT_FPS) {
        SetTargetFPS(MAX_FPS);
    }
    
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    SetExitKey(KEY_F4);

    IDButton *buttons = createButtonArray(BUTTON_COUNT, BUTTON_WIDTH, BUTTON_HEIGHT, BUTTON_BORDER_SIZE); 
    
    switch (BUTTON_COUNT) {
        case 7: setButtonColor5Fret(buttons);
                break;
        case 8: setButtonColor6Fret(buttons);
                break;
    }

    // variables

    // TODO : make screen width and height more consistent so its easier to work with
    int screenWidth = GetScreenWidth();

    IDScene currentScene = INPUT_VIEWER;

    IDGamepadState gamepadState;
    initGamepadState(&gamepadState);

    int clickedButton = -1;
    
    // beautiful loop
    while (!WindowShouldClose()) {

        // update
        if (gamepadState.gamepadNeedsRefresh) {
            setGamepadCount(&gamepadState);
            gamepadState.gamepadNeedsRefresh = 0;
        }

        if (currentScene == INPUT_VIEWER) {
            updateButtons(buttons, BUTTON_COUNT, &screenWidth, &clickedButton, &gamepadState, &currentScene);
        } else {
            updateControllerSwitcherUI(&gamepadState, &currentScene);
        }

        // render
        BeginDrawing();
            ClearBackground(BACKGROUND_COLOR);
            if (SHOW_FPS_COUNT) {
                DrawFPS(0, 0); // TODO : stupid lol
            }

            if (currentScene == INPUT_VIEWER) {
                drawButtons(buttons, BUTTON_COUNT);
            } else {
                drawControllerSwitcherUI(&gamepadState);
            }
        EndDrawing();
    }
    
    CloseWindow();

    // freeing everything
    freeBarVecs(buttons, BUTTON_COUNT);

    free(buttons);
    buttons = NULL;

    return 0;
}
