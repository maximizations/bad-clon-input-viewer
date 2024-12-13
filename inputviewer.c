// TODO : make it not suck


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

#define BUTTON_WIDTH (float)GetScreenWidth() / (float)(BUTTON_COUNT - BUTTON_BORDER_SIZE)
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
        button.pressed = false;

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


// bar update functions
IDBar createBar (IDButton *button) {
    IDBar bar;
        bar.rectangle = (Rectangle){ button->rectangle.x + button->border,
                                     button->rectangle.y + button->rectangle.height,
                                     button->rectangle.width - button->border - button->border,
                                     BAR_SPEED * GetFrameTime() };
        bar.color = button->color;
        bar.is_held = true;

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
void buttonInputReleased (IDButton *button, int *selectedGamepad) {
    if (IsGamepadButtonReleased(*selectedGamepad, button->key)) {
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

void setButtonPressed(IDButton *button, int *selectedGamepad) {
    if (IsGamepadButtonDown(*selectedGamepad, button->key)) {
        button->pressed = true;
        updateTimer(button);
    } else {
        button->pressed = false;
    }
}

void buttonInputPressed(IDButton *button, int *selectedGamepad) {
    if (IsGamepadButtonPressed(*selectedGamepad, button->key)) {
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
void setGamepadCount(int *gamepadCount, int *selectedGamepad) {
    while (IsGamepadAvailable(*gamepadCount)) {
        (*gamepadCount)++;
    }
    while (!IsGamepadAvailable(*gamepadCount) && *gamepadCount > 0) {
        (*gamepadCount)--;
        while (*selectedGamepad > *gamepadCount - 1) {
            (*selectedGamepad)--;
        }
    }
}


// big button update function
void updateButtons(IDButton *buttons, size_t arrayLength, int *screenWidth, int *clickedButton, int *selectedGamepad, IDScene *currentScene, bool *gamepadNeedsRefresh) {
    bool screenWidthChanged = 0;
    
    for (size_t i = 0; i < arrayLength; i++) {
        IDButton *currentButton = &buttons[i];

        // update the size of the buttons if the screen width changes
        if (*screenWidth != GetScreenWidth()) {
            updateButtonSize(currentButton, i);
            screenWidthChanged = 1;
        }

        if (isButtonClicked(currentButton) && *clickedButton < 0) {
            *clickedButton = i;
            currentButton->pressed = 1;
        }

        if (*clickedButton < 0) {
            setButtonPressed(currentButton, selectedGamepad);

            buttonInputPressed(currentButton, selectedGamepad);
            buttonInputReleased(currentButton, selectedGamepad);
        } else {
            IDButton *currentClickedButton = &buttons[*clickedButton];

            releaseBar(currentClickedButton);
            updateClickedButton(currentClickedButton, clickedButton);
        }

        cleanBarVec(currentButton);
        updateBarVec(currentButton);
    }

    if (screenWidthChanged) {
        *screenWidth = GetScreenWidth();
    }
    
    if (ID_KEY_BREAK) {
        *clickedButton = -1;
    }

    if (ID_KEY_ENTER && *clickedButton < 0) {
        *currentScene = CONTROLLER_SELECTOR;
        *gamepadNeedsRefresh = 1;
    }
}


// big controller switcher ui update function
void updateControllerSwitcherUI(int *gamepadCount, int *selectedGamepad, bool *gamepadNeedsRefresh, IDScene *currentScene) {
    if (ID_KEY_DOWN && *selectedGamepad < *gamepadCount - 1) {
        (*selectedGamepad)++;
        while (!IsGamepadAvailable(*selectedGamepad)) {
            (*selectedGamepad)++;
            if (*selectedGamepad >= *gamepadCount - 1) {
                break;
            }
        }
    }

    if (ID_KEY_UP && *selectedGamepad > 0) {
        (*selectedGamepad)--;
        while (!IsGamepadAvailable(*selectedGamepad)) {
            (*selectedGamepad)--;
            if (*selectedGamepad <= 0) {
                break;
            }
        }
    }
    
    if (ID_KEY_REFRESH) {
        *gamepadNeedsRefresh = 1;
    }

    if (ID_KEY_ENTER) {
        *currentScene = INPUT_VIEWER;
    }
}


// DRAWING FUNCTIONS


// on its own 
void drawFPSCount() {
    char fpscount[8];
    snprintf(fpscount, 8, "%d", GetFPS());
    
    DrawText(fpscount, GetScreenWidth() - MeasureText(fpscount, 20), 0, 20, GREEN);
}


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

void drawControllerSelectorEntry(int index, int pos, Font font, const char *gamepadName, float fontSize, float fontSpacing, int *selectedGamepad) {
    Vector2 textPosition = (Vector2){ GetScreenWidth() * 0.125f, pos * fontSize + GetScreenHeight() * 0.25f };
    
    Color textColor = WHITE; 
    
    if (index == *selectedGamepad) {
        textColor = BLUE;
        drawControllerSelectorTriangle(fontSize, textPosition, textColor);
    }

    DrawTextEx(font, gamepadName, textPosition, fontSize, fontSpacing, textColor); 
}

// TODO: THIS FUNCTION IS ASS AND HAS BASICALLY NO THOUGHT PUT INTO IT, 2 BE REWRITTEN LOL THERE IS NO LOGIC BEHIND ANYTHING 
void drawControllerSwitcherUI(int *gamepadCount, int *selectedGamepad) {
    drawControllerSelectorTitle(GetFontDefault(), clamp(min((float)GetScreenWidth(), (float)GetScreenHeight()) * 0.1f, 24.0f, 40.0f), 2.0f);

    for (int i = 0, pos = 0; i < *gamepadCount; i++, pos++) {
        const char *gamepadName = GetGamepadName(i);

        if (*gamepadName != '\0') {
            drawControllerSelectorEntry(i, pos, GetFontDefault(), gamepadName, clamp(min((float)GetScreenWidth(), (float)GetScreenHeight()) * 0.05f, 16.0f, 24.0f) , 2.0f, selectedGamepad);
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
    int screenWidth = GetScreenWidth();

    IDScene currentScene = INPUT_VIEWER;

    int clickedButton = -1;

    int gamepadCount = 0;
    int selectedGamepad = 0;
    bool gamepadNeedsRefresh = 1;
    
    // beautiful loop
    while (!WindowShouldClose()) {

        // update
        if (gamepadNeedsRefresh) {
            setGamepadCount(&gamepadCount, &selectedGamepad);
            gamepadNeedsRefresh = 0;
        }

        if (currentScene == INPUT_VIEWER) {
            updateButtons(buttons, BUTTON_COUNT, &screenWidth, &clickedButton, &selectedGamepad, &currentScene, &gamepadNeedsRefresh);
        } else {
            updateControllerSwitcherUI(&gamepadCount, &selectedGamepad, &gamepadNeedsRefresh, &currentScene);
        }

        // render
        BeginDrawing();
            ClearBackground(BACKGROUND_COLOR);
            if (SHOW_FPS_COUNT) {
                drawFPSCount();
            }

            if (currentScene == INPUT_VIEWER) {
                drawButtons(buttons, BUTTON_COUNT);
            } else {
                drawControllerSwitcherUI(&gamepadCount, &selectedGamepad);
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
