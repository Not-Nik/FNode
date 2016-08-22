// Includes
#include <stdlib.h>         // Required for: malloc(), free(), abs(), exit(), atof()
#include <stdio.h>          // Required for: FILE, fopen(), fprintf(), fclose(), fscanf(), stdout, vprintf(), sprintf(), fgets()
#include <string.h>         // Required for: strcat(), strstr()
#include <math.h>           // Required for: fabs(), sqrt(), sinf(), cosf(), cos(), sin(), tan(), pow(), floor()
#include <stdarg.h>         // Required for: va_list, va_start(), vfprintf(), va_end()
#include <raylib.h>         // Required for window management, 2D camera drawing and inputs detection

// Defines
#define     MAX_INPUTS                  4                       // Max number of inputs in every node
#define     MAX_VALUES                  16                      // Max number of values in every output
#define     MAX_NODES                   128                     // Max number of nodes
#define     MAX_NODE_LENGTH             16                      // Max node output data value text length
#define     MAX_LINES                   512                     // Max number of lines (8 lines for each node)
#define     MAX_COMMENTS                16                      // Max number of comments
#define     MAX_COMMENT_LENGTH          20                      // Max comment value text length
#define     MIN_COMMENT_SIZE            75                      // Min comment width and height values
#define     NODE_LINE_DIVISIONS         20                      // Node curved line divisions
#define     NODE_DATA_WIDTH             30                      // Node data text width
#define     NODE_DATA_HEIGHT            30                      // Node data text height
#define     UI_PADDING                  5                       // Interface bounds padding with background
#define     UI_PADDING_SCROLL           20                      // Interface scroll bar padding
#define     UI_BUTTON_HEIGHT            30                      // Interface bounds height
#define     UI_SCROLL                   20                      // Interface scroll sensitivity
#define     UI_GRID_SPACING             25                      // Interface canvas background grid divisions length
#define     UI_GRID_ALPHA               0.25f                   // Interface canvas background grid lines alpha
#define     UI_GRID_COUNT               100                     // Interface canvas background grid divisions count
#define     UI_COMMENT_WIDTH            220                     // Interface comment text box width
#define     UI_COMMENT_HEIGHT           25                      // Interface comment text box height
#define     UI_BUTTON_DEFAULT_COLOR     LIGHTGRAY               // Interface button background color
#define     UI_BORDER_DEFAULT_COLOR     125                     // Interface button border color
#define     VISOR_BORDER                2                       // Visor window border width
#define     VISOR_MODEL_ROTATION        0.0f                    // Visor model rotation speed

#define     FNODE_MALLOC(size)          malloc(size)            // Memory allocation function as define
#define     FNODE_FREE(ptr)             free(ptr)               // Memory deallocation function as define

#define     VERTEX_PATH                 "output/vertex.vs"      // Vertex shader output path
#define     FRAGMENT_PATH               "output/fragment.fs"    // Fragment shader output path
#define     DATA_PATH                   "output/shader.data"    // Created shader data output path

// Structs
typedef struct Vector4 {
    float x;
    float y;
    float z;
    float w;
} Vector4;

typedef enum {
    FNODE_PI = -2,
    FNODE_E,
    FNODE_VERTEXPOSITION,
    FNODE_VERTEXNORMAL,
    FNODE_FRESNEL,
    FNODE_VIEWDIRECTION,
    FNODE_MVP,
    FNODE_MATRIX,
    FNODE_VALUE,
    FNODE_VECTOR2,
    FNODE_VECTOR3,
    FNODE_VECTOR4,
    FNODE_ADD,
    FNODE_SUBTRACT,
    FNODE_MULTIPLY,
    FNODE_DIVIDE,
    FNODE_APPEND,
    FNODE_ONEMINUS,
    FNODE_ABS,
    FNODE_COS,
    FNODE_SIN,
    FNODE_TAN,
    FNODE_DEG2RAD,
    FNODE_RAD2DEG,
    FNODE_NORMALIZE,
    FNODE_NEGATE,
    FNODE_RECIPROCAL,
    FNODE_SQRT,
    FNODE_TRUNC,
    FNODE_ROUND,
    FNODE_CEIL,
    FNODE_CLAMP01,
    FNODE_EXP2,
    FNODE_POWER,
    FNODE_STEP,
    FNODE_POSTERIZE,
    FNODE_MAX,
    FNODE_MIN,
    FNODE_LERP,
    FNODE_SMOOTHSTEP,
    FNODE_CROSSPRODUCT,
    FNODE_DESATURATE,
    FNODE_DISTANCE,
    FNODE_DOTPRODUCT,
    FNODE_LENGTH,
    FNODE_MULTIPLYMATRIX,
    FNODE_TRANSPOSE,
    FNODE_PROJECTION,
    FNODE_REJECTION,
    FNODE_HALFDIRECTION,
    FNODE_VERTEX,
    FNODE_FRAGMENT
} FNodeType;

typedef enum {
    BUTTON_DEFAULT,
    BUTTON_HOVER,
    BUTTON_PRESSED,
    BUTTON_CLICKED
} ButtonState;

// Structs
typedef struct FNodeValue {
    float value;                            // Output data value
    Rectangle shape;                        // Output data shape
    char *valueText;                        // Output data value as text
} FNodeValue;

typedef struct FNodeOutput {
    FNodeValue data[MAX_VALUES];            // Output data array (float, Vector2, Vector3 or Vector4)
    int dataCount;                          // Output data length to know which type of data represents
} FNodeOutput;

typedef struct FNodeData {
    unsigned int id;                        // Node unique identifier
    FNodeType type;                         // Node type (values, operations...)
    const char *name;                       // Displayed name of the node
    int inputs[MAX_INPUTS];                 // Inputs node ids array
    unsigned int inputsCount;               // Inputs node ids array length
    unsigned int inputsLimit;               // Inputs node ids length limit
    FNodeOutput output;                     // Value output (contains the value and its length)
    Rectangle shape;                        // Node rectangle data
    Rectangle inputShape;                   // Node input rectangle data (automatically calculated from shape)
    Rectangle outputShape;                  // Node input rectangle data (automatically calculated from shape)
} FNodeData, *FNode;

typedef struct FLineData {
    unsigned int id;                        // Line unique identifier
    unsigned int from;                      // Id from line start node
    unsigned int to;                        // Id from line end node
} FLineData, *FLine;

typedef struct FCommentData {
    unsigned int id;                        // Comment unique identifier
    char *value;                            // Comment text label value
    Rectangle shape;                        // Comment rectangle data
    Rectangle valueShape;                   // Comment label rectangle data 
    Rectangle sizeTShape;                   // Comment top size edit rectangle data
    Rectangle sizeBShape;                   // Comment bottom size edit rectangle data
    Rectangle sizeLShape;                   // Comment left size edit rectangle data
    Rectangle sizeRShape;                   // Comment right size edit rectangle data
    Rectangle sizeTlShape;                  // Comment top-left size edit rectangle data
    Rectangle sizeTrShape;                  // Comment top-right size edit rectangle data
    Rectangle sizeBlShape;                  // Comment bottom-right size edit rectangle data
    Rectangle sizeBrShape;                  // Comment bottom-right size edit rectangle data
} FCommentData, *FComment;

// Global variables
int usedMemory = 0;                         // Total USED RAM from memory allocation

int nodesCount;                             // Created nodes count
FNode nodes[MAX_NODES];                     // Nodes pointers pool
int selectedNode = -1;                      // Current selected node to drag in canvas
int editNode = -1;                          // Current edited node data
int editNodeType = -1;                      // Current edited node data index
char *editNodeText = NULL;                  // Current edited node data value text before any changes

int linesCount;                             // Created lines count
FLine lines[MAX_LINES];                     // Lines pointers pool
int lineState = 0;                          // Current line linking state (0 = waiting for output to link, 1 = output selected, waiting for input to link)
FLine tempLine = NULL;                      // Temporally created line during line states

int commentsCount;                          // Created comments count
FComment comments[MAX_COMMENTS];            // Comments pointers pool
int commentState = 0;                       // Current comment creation state (0 = waiting to create, 1 = waiting to finish creation)
int selectedComment = -1;                   // Current selected comment to drag in canvas
int editSize = -1;                          // Current edited comment
int editSizeType = -1;                      // Current edited comment size (0 = top, 1 = bottom, 2 = left, 3 = right, 4 = top-left, 5 = top-right, 6 = bottom-left, 7 = bottom-right)
int editComment = -1;                       // Current edited comment value
int selectedCommentNodes[MAX_NODES];        // Current selected comment nodes ids list to drag
int selectedCommentNodesCount;              // Current selected comment nodes ids list count

FComment tempComment = NULL;                // Temporally created comment during comment states
Vector2 tempCommentPos = { 0, 0 };          // Temporally created comment start position

Vector2 screenSize = { 1280, 720 };         // Window screen width      
Vector2 mousePosition = { 0, 0 };           // Current mouse position 
Vector2 lastMousePosition = { 0, 0 };       // Previous frame mouse position
Vector2 mouseDelta = { 0, 0 };              // Current frame mouse position increment since previous frame
Vector2 currentOffset = { 0, 0 };           // Current selected node offset between mouse position and node shape
float modelRotation = 0.0f;                 // Current model visualization rotation angle
int scrollState = 0;                        // Current mouse drag interface scroll state

bool debugMode = false;                     // Drawing debug information state
Camera2D camera;                            // Node area 2d camera for panning
Camera camera3d;                            // Visor camera 3d for model and shader visualization
Vector2 canvasSize;                         // Interface screen size
float menuScroll = 10.0f;                   // Current interface scrolling amount
Vector2 scrollLimits = { 10, 1000 };        // Interface scrolling limits
Rectangle menuScrollRec = { 0, 0, 0, 0 };   // Interface scroll rectangle bounds
Vector2 menuScrollLimits = { 5, 685 };      // Interface scroll rectangle position limits
Rectangle canvasScroll = { 0, 0, 0, 0 };    // Interface scroll rectangle bounds
int menuOffset = 0;                         // Interface elements position current offset
Model model;                                // Visor default model for shader visualization
RenderTexture2D visorTarget;                // Visor model visualization render target
Shader fxaa;                                // Canvas and visor anti-aliasing postprocessing shader
int fxaaUniform = -1;                       // FXAA shader viewport size uniform location point
Shader shader;                              // Visor model shader
int viewUniform = -1;                       // Created shader view direction uniform location point
int transformUniform = -1;                  // Created shader model transform uniform location point
bool loadedShader = false;                  // Current loaded custom shader state
    
// Functions declarations
void UpdateMouseData();                                     // Updates current mouse position and delta position
void UpdateCanvas();                                        // Updates canvas space target and offset
void UpdateScroll();                                        // Updates mouse scrolling for menu and canvas drag
void UpdateNodesEdit();                                     // Check node data values edit input
void UpdateNodesDrag();                                     // Check node drag input
void UpdateNodesLink();                                     // Check node link input
void UpdateCommentCreationEdit();                           // Check comment creation and shape edit input
void UpdateCommentsDrag();                                  // Check comment drag input
void UpdateCommentsEdit();                                  // Check comment text edit input
void UpdateShaderData();                                    // Update required values to created shader for geometry data calculations
void CalculateValues();                                     // Calculates nodes output values based on current inputs
void CompileShader();                                       // Compiles all node structure to create the GLSL fragment shader in output folder
void CompileNode(FNode node, FILE *file, bool fragment);    // Compiles a specific node checking its inputs and writing current node operation in shader
void AlignNode(FNode node);                                 // Aligns a node to the nearest grid intersection
void AlignAllNodes();                                       // Aligns all created nodes
void ClearUnusedNodes();                                    // Destroys all unused nodes
void ClearGraph();                                          // Destroys all created nodes and its linked lines
void DrawCanvas();                                          // Draw canvas space to create nodes
void DrawCanvasGrid(int divisions);                         // Draw canvas grid with a specific number of divisions for horizontal and vertical lines
void DrawVisor();                                           // Draws a visor with default model rotating and current shader
void DrawInterface();                                       // Draw interface to create nodes

Vector2 CameraToViewVector2(Vector2 vector, Camera2D camera);   // Converts Vector2 coordinates from world space to Camera2D space based on its offset
Rectangle CameraToViewRec(Rectangle rec, Camera2D camera);      // Converts rectangle coordinates from world space to Camera2D space based on its offset

// FNode functions declarations
void InitFNode();                                                           // Initializes FNode global variables

FNode CreateNodePI();                                                       // Creates a node which returns PI value
FNode CreateNodeE();                                                        // Creates a node which returns e value
FNode CreateNodeMatrix(Matrix mat);                                         // Creates a matrix 4x4 node (OpenGL style 4x4 - right handed, column major)
FNode CreateNodeValue(float value);                                         // Creates a value node (1 float)
FNode CreateNodeVector2(Vector2 vector);                                    // Creates a Vector2 node (2 float)
FNode CreateNodeVector3(Vector3 vector);                                    // Creates a Vector3 node (3 float)
FNode CreateNodeVector4(Vector4 vector);                                    // Creates a Vector4 node (4 float)
FNode CreateNodeOperator(FNodeType type, const char *name, int inputs);     // Creates an operator node with type name and inputs limit as parameters
FNode CreateNodeUniform(FNodeType type, const char *name, int dataCount);   // Creates an uniform node with type name and data count as parameters
FNode CreateNodeMaterial(FNodeType type, const char *name, int dataCount);  // Creates the main node that contains final fragment color
FNode InitializeNode(bool isOperator);                                      // Initializes a new node with generic parameters
int GetNodeIndex(int id);                                                   // Returns the index of a node searching by its id
FLine CreateNodeLine();                                                     // Creates a line between two nodes
FComment CreateComment();                                                   // Creates a comment

void UpdateNodeShapes(FNode node);                                          // Updates a node shapes due to drag behaviour
void UpdateCommentShapes(FComment comment);                                 // Updates a comment shapes due to drag behaviour

void DrawNode(FNode node);                                                  // Draws a previously created node
void DrawNodeLine(FLine line);                                              // Draws a previously created node line
void DrawComment(FComment comment);                                         // Draws a previously created comment
bool FButton(Rectangle bounds, const char *text);                           // Button element, returns true when pressed

void DestroyNode(FNode node);                                               // Destroys a node and its linked lines
void DestroyNodeLine(FLine line);                                           // Destroys a node line
void DestroyComment(FComment comment);                                      // Destroys a comment

void CloseFNode();                                                          // Unitializes FNode global variables
void TraceLogFNode(bool error, const char *text, ...);                      // Outputs a trace log message

// FNode math functions declarations
float FVector2Length(Vector2 v);                                // Returns length of a Vector2
float FVector3Length(Vector3 v);                                // Returns length of a Vector3
float FVector4Length(Vector4 v);                                // Returns length of a Vector4
Vector2 FVector2Normalize(Vector2 v);                           // Returns a normalized Vector2
Vector3 FVector3Normalize(Vector3 v);                           // Returns a normalized Vector3
Vector4 FVector4Normalize(Vector4 v);                           // Returns a normalized Vector4
float FVector2Dot(Vector2 a, Vector2 b);                        // Returns the dot product of two Vector2
float FVector3Dot(Vector3 a, Vector3 b);                        // Returns the dot product of two Vector3
float FVector4Dot(Vector4 a, Vector4 b);                        // Returns the dot product of two Vector4
Vector2 FVector2Projection(Vector2 a, Vector2 b);               // Returns the projection vector of two Vector2
Vector3 FVector3Projection(Vector3 a, Vector3 b);               // Returns the projection vector of two Vector3
Vector4 FVector4Projection(Vector4 a, Vector4 b);               // Returns the projection vector of two Vector4
Vector2 FVector2Rejection(Vector2 a, Vector2 b);                // Returns the rejection vector of two Vector2
Vector3 FVector3Rejection(Vector3 a, Vector3 b);                // Returns the rejection vector of two Vector3
Vector4 FVector4Rejection(Vector4 a, Vector4 b);                // Returns the rejection vector of two Vector4
Vector3 FCrossProduct(Vector3 a, Vector3 b);                    // Returns the cross product of two vectors
Matrix FMatrixIdentity();                                       // Returns identity matrix
Matrix FMatrixMultiply(Matrix left, Matrix right);              // Returns the result of multiply two matrices
Matrix FMatrixTranslate(float x, float y, float z);             // Returns translation matrix
Matrix FMatrixRotate(Vector3 axis, float angle);                // Create rotation matrix from axis and angle provided in radians
Matrix FMatrixScale(float x, float y, float z);                 // Returns scaling matrix
void FMatrixTranspose(Matrix *mat);                             // Transposes provided matrix
void FMultiplyMatrixVector(Vector4 *v, Matrix mat);             // Transform a quaternion given a transformation matrix
float FCos(float value);                                        // Returns the cosine value of a radian angle
float FSin(float value);                                        // Returns the sine value of a radian angle
float FTan(float value);                                        // Returns the tangent value of a radian angle
float FPower(float value, float exp);                           // Returns a value to the power of an exponent
float FSquareRoot(float value);                                 // Returns the square root of the input value
float FPosterize(float value, float samples);                   // Returns a value rounded based on the samples
float FClamp(float value, float min, float max);                // Returns a value clamped by a min and max values
float FTrunc(float value);                                      // Returns a truncated value of a value
float FRound(float value);                                      // Returns a rounded value of a value
float FCeil(float value);                                       // Returns a rounded up to the nearest integer of a value
float FLerp(float valueA, float valueB, float time);            // Returns the interpolation between two values
float FSmoothStep(float min, float max, float value);           // Returns the interpolate of a value in a range
float FEaseLinear(float t, float b, float c, float d);          // Returns an ease linear value between two parameters 
float FEaseInOutQuad(float t, float b, float c, float d);       // Returns an ease quadratic in-out value between two parameters

// FNode string functions declarations
void FStringToFloat(float *pointer, const char *string);        // Sends a float conversion value of a string to an initialized float pointer
void FFloatToString(char *buffer, float value);                 // Sends formatted output to an initialized string pointer
int FSearch(char *filename, char *string);                      // Returns 1 if a specific string is found in a text file

int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
    InitWindow(screenSize.x, screenSize.y, "fnode 1.0");
    SetTargetFPS(60);
    SetLineWidth(3);

    // Load resources
    model = LoadModel("res/model.obj");
    visorTarget = LoadRenderTexture(screenSize.x/4, screenSize.y/4);
    fxaa = LoadShader("res/fxaa.vs", "res/fxaa.fs");

    // Initialize values
    camera = (Camera2D){ (Vector2){ 0, 0 }, (Vector2){ screenSize.x/2, screenSize.y/2 }, 0.0f, 1.0f };
    canvasSize = (Vector2){ screenSize.x*0.85f, screenSize.y };
    camera3d = (Camera){{ 0.0f, 0.0f, 4.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, 45.0f };
    menuScrollRec = (Rectangle){ screenSize.x - 17, 5, 9, 30 };
    
    // Initialize shaders values
    fxaaUniform = GetShaderLocation(fxaa, "viewportSize");
    float viewportSize[2] = { screenSize.x/4, screenSize.y/4 };
    SetShaderValue(fxaa, fxaaUniform, viewportSize, 2);

    // Setup orbital camera
    SetCameraPosition(camera3d.position);     // Set internal camera position to match our camera position
    SetCameraTarget(camera3d.target);         // Set internal camera target to match our camera target

    InitFNode();
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        UpdateMouseData();
        UpdateCanvas();
        UpdateScroll();
        UpdateNodesEdit();
        UpdateNodesDrag();
        UpdateNodesLink();
        UpdateCommentCreationEdit();
        UpdateCommentsEdit();
        UpdateCommentsDrag();
        UpdateShaderData();

        if (IsKeyPressed('P')) debugMode = !debugMode;
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            DrawCanvas();

            DrawInterface();
            
            DrawVisor();

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadModel(model);
    UnloadRenderTexture(visorTarget);
    if (loadedShader) UnloadShader(shader);
    UnloadShader(fxaa);

    CloseFNode();
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

// Updates current mouse position and delta position
void UpdateMouseData()
{
    // Update mouse position values
    lastMousePosition = mousePosition;
    mousePosition = GetMousePosition();
    mouseDelta = (Vector2){ mousePosition.x - lastMousePosition.x, mousePosition.y - lastMousePosition.y };
}

// Updates canvas space target and offset
void UpdateCanvas()
{
    // Update canvas camera values
    camera.target = mousePosition;
    
    // Update visor model current rotation
    modelRotation -= VISOR_MODEL_ROTATION;
}

// Updates mouse scrolling for menu and canvas drag
void UpdateScroll()
{
    // Check zoom input
    if (GetMouseWheelMove() != 0)
    {
        if (CheckCollisionPointRec(mousePosition, (Rectangle){ canvasSize.x - visorTarget.texture.width - UI_PADDING, screenSize.y - visorTarget.texture.height - UI_PADDING, visorTarget.texture.width, visorTarget.texture.height }))
        {
            camera3d.position.z += GetMouseWheelMove()*0.25f;
            camera3d.position.z = FClamp(camera3d.position.z, 2.5f, 6.0f);
        }
        else if (CheckCollisionPointRec(mousePosition, (Rectangle){ 0, 0, canvasSize.x, canvasSize.y }))
        {
            if (IsKeyDown(KEY_LEFT_ALT)) camera.offset.x -= GetMouseWheelMove()*UI_SCROLL;
            else camera.offset.y -= GetMouseWheelMove()*UI_SCROLL;
        }
        else
        {
            menuScroll -= GetMouseWheelMove()*UI_SCROLL;
            menuScroll = FClamp(menuScroll, scrollLimits.x, scrollLimits.y);
            menuScrollRec.y = (menuScrollLimits.y - menuScrollLimits.x)*menuScroll/(scrollLimits.y - scrollLimits.x);
        }
    }
    
    // Check mouse drag interface scrolling input
    if (scrollState == 0)
    {
        if ((IsMouseButtonDown(MOUSE_LEFT_BUTTON)) && (CheckCollisionPointRec(mousePosition, menuScrollRec))) scrollState = 1;
    }
    else
    {
        menuScroll += mouseDelta.y*1.45f;
        menuScrollRec.y += mouseDelta.y;
        
        if (menuScrollRec.y >= menuScrollLimits.y)
        {
            menuScroll = scrollLimits.y;
            menuScrollRec.y = menuScrollLimits.y;
        }
        else if (menuScrollRec.y <= menuScrollLimits.x)
        {
            menuScroll = scrollLimits.x;
            menuScrollRec.y = menuScrollLimits.x;
        }

        if (IsMouseButtonUp(MOUSE_LEFT_BUTTON)) scrollState = 0;
    }
}

// Check node data values edit input
void UpdateNodesEdit()
{
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        int index = -1;
        int data = -1;
        for (int i = 0; i < nodesCount; i++)
        {
            if ((nodes[i]->type >= FNODE_MATRIX) && (nodes[i]->type <= FNODE_VECTOR4))
            {
                for (int k = 0; k < nodes[i]->output.dataCount; k++)
                {
                    if (CheckCollisionPointRec(mousePosition, CameraToViewRec(nodes[i]->output.data[k].shape, camera)))
                    {
                        index = i;
                        data = k;
                        break;
                    }
                }
            }
        }

        if (index != -1)
        {
            if ((editNode == -1) && (selectedNode == -1) && (lineState == 0) && (commentState == 0) && (selectedComment == -1) && (editSize == -1) && (editSizeType == -1) && (editComment == -1))
            {
                editNode = nodes[index]->id;
                editNodeType = data;
                editNodeText = (char *)FNODE_MALLOC(MAX_NODE_LENGTH);
                usedMemory += MAX_NODE_LENGTH;
                for (int i = 0; i < MAX_NODE_LENGTH; i++) editNodeText[i] = nodes[index]->output.data[data].valueText[i];
            }
            else if ((editNode != -1) && (selectedNode == -1) && (lineState == 0) && (commentState == 0) && (selectedComment == -1) && (editSize == -1) && (editSizeType == -1) && (editComment == -1))
            {
                if ((nodes[index]->id != editNode) || (data != editNodeType))
                {
                    for (int i = 0; i < nodesCount; i++)
                    {
                        if (nodes[i]->id == editNode)
                        {
                            for (int k = 0; k < MAX_NODE_LENGTH; k++) nodes[i]->output.data[editNodeType].valueText[k] = editNodeText[k];
                        }
                    }

                    editNode = nodes[index]->id;
                    editNodeType = data;

                    for (int i = 0; i < MAX_NODE_LENGTH; i++) editNodeText[i] = nodes[index]->output.data[data].valueText[i];
                }
            }
        }
        else if ((editNode != -1) && (editNodeType != -1))
        {
            for (int i = 0; i < nodesCount; i++)
            {
                if (nodes[i]->id == editNode)
                {
                    for (int k = 0; k < MAX_NODE_LENGTH; k++) nodes[i]->output.data[editNodeType].valueText[k] = editNodeText[k];
                }
            }

            editNode = -1;
            editNodeType = -1;
            FNODE_FREE(editNodeText);
            usedMemory -= MAX_NODE_LENGTH;
            editNodeText = NULL;
        }     
    }
}

// Check node drag input
void UpdateNodesDrag()
{
    if ((selectedNode == -1) && (lineState == 0) && (commentState == 0) && (selectedComment == -1))
    {
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
        {
            for (int i = nodesCount - 1; i >= 0; i--)
            {
                if (CheckCollisionPointRec(mousePosition, CameraToViewRec(nodes[i]->shape, camera)))
                {
                    selectedNode = nodes[i]->id;
                    currentOffset = (Vector2){ mousePosition.x - nodes[i]->shape.x, mousePosition.y - nodes[i]->shape.y };
                    break;
                }
            }

            if ((selectedNode == -1) && (scrollState == 0) && (!CheckCollisionPointRec(mousePosition, (Rectangle){ canvasSize.x, 0, (screenSize.x - canvasSize.x), screenSize.y })))
            {
                camera.offset.x += mouseDelta.x;
                camera.offset.y += mouseDelta.y;
            }
        }
        else if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON))
        {
            for (int i = nodesCount - 1; i >= 0; i--)
            {
                if (CheckCollisionPointRec(mousePosition, CameraToViewRec(nodes[i]->shape, camera)) && (nodes[i]->type < FNODE_VERTEX))
                {
                    DestroyNode(nodes[i]);
                    CalculateValues();
                    break;
                }
            }
        }
    }
    else if ((selectedNode != -1) && (lineState == 0) && (commentState == 0) && (selectedComment == -1))
    {
        for (int i = 0; i < nodesCount; i++)
        {
            if (nodes[i]->id == selectedNode)
            {
                nodes[i]->shape.x = mousePosition.x - currentOffset.x;
                nodes[i]->shape.y = mousePosition.y - currentOffset.y;

                // Check aligned drag movement input
                if (IsKeyDown(KEY_LEFT_ALT)) AlignNode(nodes[i]);

                UpdateNodeShapes(nodes[i]);
                break;
            }
        }

        if (IsMouseButtonUp(MOUSE_LEFT_BUTTON)) selectedNode = -1;
    }
}

// Check node link input
void UpdateNodesLink()
{
    if ((selectedNode == -1) && (commentState == 0) && (selectedComment == -1)) 
    {
        switch (lineState)
        {
            case 0:
            {
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                {
                    for (int i = nodesCount - 1; i >= 0; i--)
                    {
                        if (CheckCollisionPointRec(mousePosition, CameraToViewRec(nodes[i]->outputShape, camera)))
                        {
                            tempLine = CreateNodeLine(nodes[i]->id);
                            lineState = 1;
                            break;
                        }
                    }
                }
                else if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON) && !IsKeyDown(KEY_LEFT_ALT))
                {
                    for (int i = nodesCount - 1; i >= 0; i--)
                    {
                        if (CheckCollisionPointRec(mousePosition, CameraToViewRec(nodes[i]->outputShape, camera)))
                        {
                            for (int k = linesCount - 1; k >= 0; k--)
                            {
                                if (nodes[i]->id == lines[k]->from) DestroyNodeLine(lines[k]);
                            }

                            CalculateValues();
                            CalculateValues();
                            break;
                        }
                        else if (CheckCollisionPointRec(mousePosition, CameraToViewRec(nodes[i]->inputShape, camera)))
                        {
                            for (int k = linesCount - 1; k >= 0; k--)
                            {
                                if (nodes[i]->id == lines[k]->to) DestroyNodeLine(lines[k]);
                            }

                            CalculateValues();
                            CalculateValues();
                            break;
                        }
                    }
                }
            } break;
            case 1:
            {
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                {
                    for (int i = 0; i < nodesCount; i++)
                    {
                        if (CheckCollisionPointRec(mousePosition, CameraToViewRec(nodes[i]->inputShape, camera)) && (nodes[i]->id != tempLine->from) && (nodes[i]->inputsCount < nodes[i]->inputsLimit))
                        {
                            // Get which index has the first input node id from current nude                            
                            int indexFrom = GetNodeIndex(tempLine->from);

                            bool valuesCheck = true;
                            if (nodes[i]->type == FNODE_APPEND) valuesCheck = ((nodes[i]->output.dataCount + nodes[indexFrom]->output.dataCount <= 4) && (nodes[indexFrom]->output.dataCount == 1));
                            else if ((nodes[i]->type == FNODE_POWER) && (nodes[i]->inputsCount == 1)) valuesCheck = (nodes[indexFrom]->output.dataCount == 1);
                            else if (nodes[i]->type == FNODE_STEP) valuesCheck = (nodes[indexFrom]->output.dataCount == 1);
                            else if (nodes[i]->type == FNODE_NORMALIZE) valuesCheck = ((nodes[indexFrom]->output.dataCount > 1) && (nodes[indexFrom]->output.dataCount <= 4));
                            else if (nodes[i]->type == FNODE_CROSSPRODUCT) valuesCheck = (nodes[indexFrom]->output.dataCount == 3);
                            else if (nodes[i]->type == FNODE_DESATURATE)
                            {
                                if (nodes[i]->inputsCount == 0) valuesCheck = (nodes[indexFrom]->output.dataCount < 4);
                                else if (nodes[i]->inputsCount == 1) valuesCheck = (nodes[indexFrom]->output.dataCount == 1);
                            }
                            else if ((nodes[i]->type == FNODE_DOTPRODUCT) || (nodes[i]->type == FNODE_LENGTH) || ((nodes[i]->type >= FNODE_PROJECTION) && (nodes[i]->type <= FNODE_HALFDIRECTION)))
                            {
                                valuesCheck = ((nodes[indexFrom]->output.dataCount > 1) && (nodes[indexFrom]->output.dataCount <= 4));

                                if (valuesCheck && (nodes[i]->inputsCount > 0))
                                {
                                    int index = GetNodeIndex(nodes[i]->inputs[0]);
                                    
                                    if (index != -1) valuesCheck = (nodes[indexFrom]->output.dataCount == nodes[index]->output.dataCount);
                                    else TraceLogFNode(true, "error when trying to get node inputs index");
                                }
                            }
                            else if (nodes[i]->type == FNODE_DISTANCE)
                            {
                                valuesCheck = ((nodes[indexFrom]->output.dataCount <= 4));

                                if (valuesCheck && (nodes[i]->inputsCount > 0))
                                {
                                    int index = GetNodeIndex(nodes[i]->inputs[0]);
                                    
                                    if (index != -1) valuesCheck = (nodes[indexFrom]->output.dataCount == nodes[index]->output.dataCount);
                                    else TraceLogFNode(true, "error when trying to get node inputs index");
                                }
                            }
                            else if ((nodes[i]->type == FNODE_MULTIPLYMATRIX) || (nodes[i]->type == FNODE_TRANSPOSE)) valuesCheck = (nodes[indexFrom]->output.dataCount == 16);
                            else if (nodes[i]->type >= FNODE_VERTEX) valuesCheck = (nodes[indexFrom]->output.dataCount <= nodes[i]->output.dataCount);
                            else if (nodes[i]->type > FNODE_DIVIDE) valuesCheck = (nodes[i]->output.dataCount == nodes[indexFrom]->output.dataCount);

                            if (((nodes[i]->inputsCount == 0) && (nodes[i]->type != FNODE_NORMALIZE) && (nodes[i]->type != FNODE_DOTPRODUCT) && 
                            (nodes[i]->type != FNODE_LENGTH) && (nodes[i]->type != FNODE_MULTIPLYMATRIX) && (nodes[i]->type != FNODE_TRANSPOSE) && (nodes[i]->type != FNODE_PROJECTION) &&
                            (nodes[i]->type != FNODE_DISTANCE) && (nodes[i]->type != FNODE_REJECTION) && (nodes[i]->type != FNODE_HALFDIRECTION) && (nodes[i]->type != FNODE_STEP)) || valuesCheck)
                            {
                                // Check if there is already a line created with same linking ids
                                for (int k = 0; k < linesCount; k++)
                                {
                                    if ((lines[k]->to == nodes[i]->id) && (lines[k]->from == tempLine->from))
                                    {
                                        DestroyNodeLine(lines[k]);
                                        break;
                                    }
                                }

                                // Save temporal line values and destroy it
                                int from = tempLine->from;
                                int to = nodes[i]->id;
                                DestroyNodeLine(tempLine);

                                // Create final node line
                                FLine temp = CreateNodeLine(from);
                                temp->to = to;

                                // Reset linking state values
                                lineState = 0;
                                CalculateValues();
                                CalculateValues();
                                break;
                            }
                            else TraceLogFNode(false, "error trying to link node ID %i (length: %i) with node ID %i (length: %i)", nodes[i]->id, nodes[i]->output.dataCount, nodes[indexFrom]->id, nodes[indexFrom]->output.dataCount);
                        }
                    }
                }
                else if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON))
                {
                    DestroyNodeLine(tempLine);
                    lineState = 0;
                }
            } break;
            default: break;
        }
    }
}

// Check comment creation input
void UpdateCommentCreationEdit()
{
    if ((selectedNode == -1) && (lineState == 0) && (selectedComment == -1)) 
    {
        switch (commentState)
        {
            case 0:
            {
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                {
                    if (IsKeyDown(KEY_LEFT_ALT))
                    {
                        commentState = 1;

                        tempCommentPos.x = mousePosition.x;
                        tempCommentPos.y = mousePosition.y;

                        tempComment = CreateComment();
                        tempComment->shape.x = mousePosition.x - camera.offset.x;
                        tempComment->shape.y = mousePosition.y - camera.offset.y;

                        UpdateCommentShapes(tempComment);
                    }
                    else
                    {
                        for (int i = 0; i < commentsCount; i++)
                        {
                            if (CheckCollisionPointRec(mousePosition, CameraToViewRec(comments[i]->sizeTShape, camera)))
                            {
                                editSize = comments[i]->id;
                                commentState = 1;
                                editSizeType = 0;
                                break;
                            }
                            else if (CheckCollisionPointRec(mousePosition, CameraToViewRec(comments[i]->sizeBShape, camera)))
                            {
                                editSize = comments[i]->id;
                                commentState = 1;
                                editSizeType = 1;
                                break;
                            }
                            else if (CheckCollisionPointRec(mousePosition, CameraToViewRec(comments[i]->sizeLShape, camera)))
                            {
                                editSize = comments[i]->id;
                                commentState = 1;
                                editSizeType = 2;
                                break;
                            }
                            else if (CheckCollisionPointRec(mousePosition, CameraToViewRec(comments[i]->sizeRShape, camera)))
                            {
                                editSize = comments[i]->id;
                                commentState = 1;
                                editSizeType = 3;
                                break;
                            }
                            else if (CheckCollisionPointRec(mousePosition, CameraToViewRec(comments[i]->sizeTlShape, camera)))
                            {
                                editSize = comments[i]->id;
                                commentState = 1;
                                editSizeType = 4;
                                break;
                            }
                            else if (CheckCollisionPointRec(mousePosition, CameraToViewRec(comments[i]->sizeTrShape, camera)))
                            {
                                editSize = comments[i]->id;
                                commentState = 1;
                                editSizeType = 5;
                                break;
                            }
                            else if (CheckCollisionPointRec(mousePosition, CameraToViewRec(comments[i]->sizeBlShape, camera)))
                            {
                                editSize = comments[i]->id;
                                commentState = 1;
                                editSizeType = 6;
                                break;
                            }
                            else if (CheckCollisionPointRec(mousePosition, CameraToViewRec(comments[i]->sizeBrShape, camera)))
                            {
                                editSize = comments[i]->id;
                                commentState = 1;
                                editSizeType = 7;
                                break;
                            }
                        }
                    }
                }
            } break;
            case 1:
            {
                if (editSize != -1)
                {
                    for (int i = 0; i < commentsCount; i++)
                    {
                        if (comments[i]->id == editSize)
                        {
                            switch (editSizeType)
                            {
                                case 0:
                                {
                                    comments[i]->shape.y += mouseDelta.y;
                                    comments[i]->shape.height -= mouseDelta.y;
                                } break;
                                case 1: comments[i]->shape.height += mouseDelta.y; break;
                                case 2:
                                {
                                    comments[i]->shape.x += mouseDelta.x;
                                    comments[i]->shape.width -= mouseDelta.x;
                                } break;
                                case 3: comments[i]->shape.width += mouseDelta.x; break;
                                case 4:
                                {
                                    comments[i]->shape.x += mouseDelta.x;
                                    comments[i]->shape.width -= mouseDelta.x;
                                    comments[i]->shape.y += mouseDelta.y;
                                    comments[i]->shape.height -= mouseDelta.y;
                                } break;
                                case 5:
                                {
                                    comments[i]->shape.width += mouseDelta.x;
                                    comments[i]->shape.y += mouseDelta.y;
                                    comments[i]->shape.height -= mouseDelta.y;
                                } break;
                                case 6:
                                {
                                    comments[i]->shape.x += mouseDelta.x;
                                    comments[i]->shape.width -= mouseDelta.x;
                                    comments[i]->shape.height += mouseDelta.y;
                                } break;
                                case 7:
                                {
                                    comments[i]->shape.width += mouseDelta.x;
                                    comments[i]->shape.height += mouseDelta.y;
                                } break;
                                default: break;
                            }

                            UpdateCommentShapes(comments[i]);
                            break;
                        }
                    }

                    if (IsMouseButtonUp(MOUSE_LEFT_BUTTON))
                    {
                        editSize = -1;
                        editSizeType = -1;
                        commentState = 0;
                    }
                }
                else
                {
                    if ((mousePosition.x - tempCommentPos.x) >= 0) tempComment->shape.width = mousePosition.x - tempComment->shape.x - camera.offset.x;
                    else
                    {
                        tempComment->shape.width = tempCommentPos.x - mousePosition.x;
                        tempComment->shape.x = tempCommentPos.x - tempComment->shape.width - camera.offset.x;
                    }

                    if ((mousePosition.y - tempCommentPos.y) >= 0) tempComment->shape.height = mousePosition.y - tempComment->shape.y - camera.offset.y;
                    else
                    {
                        tempComment->shape.height = tempCommentPos.y - mousePosition.y;
                        tempComment->shape.y = tempCommentPos.y - tempComment->shape.height - camera.offset.y;
                    }

                    UpdateCommentShapes(tempComment);

                    if (IsMouseButtonUp(MOUSE_LEFT_BUTTON))
                    {
                        // Save temporal comment values
                        Rectangle tempRec = { tempComment->shape.x, tempComment->shape.y, tempComment->shape.width, tempComment->shape.height };
                        DestroyComment(tempComment);

                        // Reset comment state
                        commentState = 0;

                        if (tempRec.width >= 0 && tempRec.height >= 0)
                        {
                            // Create final comment
                            FComment temp = CreateComment();
                            temp->shape = tempRec;
                            
                            UpdateCommentShapes(temp);
                        }
                        else TraceLogFNode(false, "comment have not been created because its width or height are has a negative value");
                    }
                }
            } break;
            default: break;
        }
    }
}

// Check comment drag input
void UpdateCommentsDrag()
{
    if ((selectedComment == -1) && (lineState == 0) && (commentState == 0) && (selectedNode == -1))
    {
        if (!IsKeyDown(KEY_LEFT_ALT))
        {
            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
            {
                for (int i = commentsCount - 1; i >= 0; i--)
                {
                    if (CheckCollisionPointRec(mousePosition, CameraToViewRec(comments[i]->shape, camera)))
                    {
                        selectedComment = comments[i]->id;
                        currentOffset = (Vector2){ mousePosition.x - comments[i]->shape.x, mousePosition.y - comments[i]->shape.y };

                        for (int k = 0; k < nodesCount; k++)
                        {
                            if (CheckCollisionRecs(CameraToViewRec(comments[i]->shape, camera), CameraToViewRec(nodes[k]->shape, camera)))
                            {
                                selectedCommentNodes[selectedCommentNodesCount] = nodes[k]->id;
                                selectedCommentNodesCount++;
                                
                                if (selectedCommentNodesCount > MAX_NODES) break;
                            }
                        }

                        break;
                    }
                }
            }
        }
        else if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON))
        {
            for (int i = commentsCount - 1; i >= 0; i--)
            {
                if (CheckCollisionPointRec(mousePosition, CameraToViewRec(comments[i]->shape, camera)))
                {
                    DestroyComment(comments[i]);
                    break;
                }
            }
        }
    }
    else if ((selectedComment != -1) && (lineState == 0) && (commentState == 0) && (selectedNode == -1))
    {
        for (int i = 0; i < commentsCount; i++)
        {
            if (comments[i]->id == selectedComment)
            {
                comments[i]->shape.x = mousePosition.x - currentOffset.x;
                comments[i]->shape.y = mousePosition.y - currentOffset.y;

                UpdateCommentShapes(comments[i]);

                for (int k = 0; k < selectedCommentNodesCount; k++)
                {
                    for (int j = 0; j < nodesCount; j++)
                    {
                        if (nodes[j]->id == selectedCommentNodes[k])
                        {
                            nodes[j]->shape.x += mouseDelta.x;
                            nodes[j]->shape.y += mouseDelta.y;
                            
                            UpdateNodeShapes(nodes[j]);
                            break;
                        }
                    }
                }
                break;
            }
        }

        if (IsMouseButtonUp(MOUSE_LEFT_BUTTON))
        {
            selectedComment = -1;
            
            for (int i = 0; i < selectedCommentNodesCount; i++) selectedCommentNodes[i] = -1;
            selectedCommentNodesCount = 0;
        }
    }
}

// Check comment text edit input
void UpdateCommentsEdit()
{
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        if ((editComment == -1) && (selectedNode == -1) && (lineState == 0) && (commentState == 0) && (selectedComment == -1) && (editSize == -1) && (editSizeType == -1) && (editNode == -1))
        {
            for (int i = 0; i < commentsCount; i++)
            {
                if (CheckCollisionPointRec(mousePosition, CameraToViewRec(comments[i]->valueShape, camera)))
                {
                    editComment = i;
                    break;
                }
            }
        }
        else if ((editComment != -1) && (selectedNode == -1) && (lineState == 0) && (commentState == 0) && (selectedComment == -1) && (editSize == -1) && (editSizeType == -1) && (editNode == -1))
        {
            bool isCurrentText = false;
            int currentEdit = editComment;
            for (int i = 0; i < commentsCount; i++)
            {
                if (comments[i]->id == editComment)
                {
                    if (CheckCollisionPointRec(mousePosition, CameraToViewRec(comments[i]->valueShape, camera)))
                    {
                        isCurrentText = true;
                        break;
                    }
                }

                if (CheckCollisionPointRec(mousePosition, CameraToViewRec(comments[i]->valueShape, camera)))
                {
                    editComment = i;
                    break;
                }
            }

            // Reset current editing text any other text label is pressed
            if (!isCurrentText && (currentEdit == editComment)) editComment = -1;
        }
    }
}

// Update required values to created shader for geometry data calculations
void UpdateShaderData()
{
    if (shader.id != 0)
    {
        Vector3 viewVector = { camera3d.position.x - camera3d.target.x, camera3d.position.y - camera3d.target.y, camera3d.position.z - camera3d.target.z };
        viewVector = FVector3Normalize(viewVector);
        float viewDir[3] = {  viewVector.x, viewVector.y, viewVector.z };
        SetShaderValue(shader, viewUniform, viewDir, 3);
        
        SetShaderValueMatrix(shader, transformUniform, model.transform);
    }
}

// Calculates nodes output values based on current inputs
void CalculateValues()
{
    for (int i = 0; i < nodesCount; i++)
    {
        if (nodes[i] != NULL)
        {
            // Reset node inputs values and inputs count
            for (int k = 0; k < MAX_INPUTS; k++) nodes[i]->inputs[k] = -1;
            nodes[i]->inputsCount = 0;

            // Search for node inputs and calculate inputs count
            for (int k = 0; k < linesCount; k++)
            {
                if (lines[k] != NULL)
                {
                    // Check if line output (to) is the node input id
                    if (lines[k]->to == nodes[i]->id)
                    {
                        nodes[i]->inputs[nodes[i]->inputsCount] = lines[k]->from;
                        nodes[i]->inputsCount++;
                    }
                }
            }

            // Check if current node is an operator
            if (nodes[i]->type > FNODE_VECTOR4 && nodes[i]->type < FNODE_APPEND)
            {
                // Calculate output values count based on first input node value count
                if (nodes[i]->inputsCount > 0) 
                {
                    // Get which index has the first input node id from current nude
                    int index = GetNodeIndex(nodes[i]->inputs[0]);
                    
                    nodes[i]->output.dataCount = nodes[index]->output.dataCount;
                    for (int k = 0; k < nodes[i]->output.dataCount; k++) nodes[i]->output.data[k].value = nodes[index]->output.data[k].value;

                    for (int k = 1; k < nodes[i]->inputsCount; k++)
                    {
                        int inputIndex = GetNodeIndex(nodes[i]->inputs[k]);

                        // Apply inputs values to output based on node operation type
                        switch (nodes[i]->type)
                        {
                            case FNODE_ADD:
                            {
                                for (int j = 0; j < nodes[i]->output.dataCount; j++) nodes[i]->output.data[j].value += nodes[inputIndex]->output.data[j].value;
                            } break;
                            case FNODE_SUBTRACT:
                            {
                                for (int j = 0; j < nodes[i]->output.dataCount; j++) nodes[i]->output.data[j].value -= nodes[inputIndex]->output.data[j].value;
                            } break;
                            case FNODE_MULTIPLY:
                            {
                                if (nodes[inputIndex]->output.dataCount == 1)
                                {
                                    for (int j = 0; j < nodes[i]->output.dataCount; j++) nodes[i]->output.data[j].value *= nodes[inputIndex]->output.data[0].value;
                                }
                                else
                                {
                                    for (int j = 0; j < nodes[i]->output.dataCount; j++)
                                    {
                                        if (nodes[inputIndex]->output.dataCount == 16 && nodes[i]->output.dataCount == 3)
                                        {
                                            Vector4 vector = { nodes[i]->output.data[0].value, nodes[i]->output.data[1].value, nodes[i]->output.data[2].value, 1.0f };
                                            Matrix matrix = { nodes[inputIndex]->output.data[4].value, nodes[inputIndex]->output.data[5].value, nodes[inputIndex]->output.data[6].value, nodes[inputIndex]->output.data[7].value,
                                            nodes[inputIndex]->output.data[8].value, nodes[inputIndex]->output.data[9].value, nodes[inputIndex]->output.data[10].value, nodes[inputIndex]->output.data[11].value,
                                            nodes[inputIndex]->output.data[12].value, nodes[inputIndex]->output.data[13].value, nodes[inputIndex]->output.data[14].value, nodes[inputIndex]->output.data[15].value };
                                            FMultiplyMatrixVector(&vector, matrix);
                                            
                                            for (int k = 0; k < MAX_VALUES; k++) nodes[i]->output.data[k].value = 0.0f;
                                            nodes[i]->output.data[0].value = vector.x;
                                            nodes[i]->output.data[1].value = vector.y;
                                            nodes[i]->output.data[2].value = vector.z;
                                            nodes[i]->output.data[3].value = vector.w;
                                            nodes[i]->output.dataCount = 4;
                                        }
                                        else if (nodes[inputIndex]->output.dataCount == 3 && nodes[i]->output.dataCount == 16)
                                        {
                                            Vector4 vector = { nodes[inputIndex]->output.data[0].value, nodes[inputIndex]->output.data[1].value, nodes[inputIndex]->output.data[2].value, 1.0f };
                                            Matrix matrix = { nodes[i]->output.data[4].value, nodes[i]->output.data[5].value, nodes[i]->output.data[6].value, nodes[i]->output.data[7].value,
                                            nodes[i]->output.data[8].value, nodes[i]->output.data[9].value, nodes[i]->output.data[10].value, nodes[i]->output.data[11].value,
                                            nodes[i]->output.data[12].value, nodes[i]->output.data[13].value, nodes[i]->output.data[14].value, nodes[i]->output.data[15].value };
                                            FMultiplyMatrixVector(&vector, matrix);
                                            
                                            for (int k = 0; k < MAX_VALUES; k++) nodes[i]->output.data[k].value = 0.0f;
                                            nodes[i]->output.data[0].value = vector.x;
                                            nodes[i]->output.data[1].value = vector.y;
                                            nodes[i]->output.data[2].value = vector.z;
                                            nodes[i]->output.data[3].value = vector.w;
                                            nodes[i]->output.dataCount = 4;
                                        }
                                        else nodes[i]->output.data[j].value *= nodes[inputIndex]->output.data[j].value;
                                    }
                                }
                            } break;
                            case FNODE_DIVIDE:
                            {
                                if (nodes[inputIndex]->output.dataCount == 1)
                                {
                                    for (int j = 0; j < nodes[i]->output.dataCount; j++) nodes[i]->output.data[j].value /= nodes[inputIndex]->output.data[0].value;
                                }
                                else
                                {
                                    for (int j = 0; j < nodes[i]->output.dataCount; j++) nodes[i]->output.data[j].value /= nodes[inputIndex]->output.data[j].value;
                                }
                            } break;
                            default: break;
                        }
                    }
                }
                else
                {
                    for (int k = 0; k < MAX_VALUES; k++) nodes[i]->output.data[k].value = 0.0f;
                    nodes[i]->output.dataCount = 0;
                }
            }
            else if (nodes[i]->type == FNODE_APPEND)
            {
                if (nodes[i]->inputsCount > 0)
                {
                    int valuesCount = 0;
                    for (int k = 0; k < nodes[i]->inputsCount; k++)
                    {
                        int inputIndex = GetNodeIndex(nodes[i]->inputs[k]);

                        for (int j = 0; j < nodes[inputIndex]->output.dataCount; j++)
                        {
                            nodes[i]->output.data[valuesCount].value = nodes[inputIndex]->output.data[j].value;
                            valuesCount++;
                        }
                    }

                    nodes[i]->output.dataCount = valuesCount;
                }
                else
                {
                    for (int k = 0; k < MAX_VALUES; k++) nodes[i]->output.data[k].value = 0.0f;
                    nodes[i]->output.dataCount = 0;
                }
            }
            else if (nodes[i]->type > FNODE_APPEND)
            {
                if (nodes[i]->inputsCount > 0)
                {
                    int index = GetNodeIndex(nodes[i]->inputs[0]);

                    nodes[i]->output.dataCount = nodes[index]->output.dataCount;
                    for (int k = 0; k < nodes[i]->output.dataCount; k++) nodes[i]->output.data[k].value = nodes[index]->output.data[k].value;

                    switch (nodes[i]->type)
                    {
                        case FNODE_ONEMINUS:
                        {
                            for (int j = 0; j < nodes[i]->output.dataCount; j++) nodes[i]->output.data[j].value = 1 - nodes[i]->output.data[j].value;
                        } break;
                        case FNODE_ABS:
                        {
                            for (int j = 0; j < nodes[i]->output.dataCount; j++)
                            {
                                if (nodes[i]->output.data[j].value < 0) nodes[i]->output.data[j].value *= -1;
                            }
                        } break;
                        case FNODE_COS:
                        {
                            for (int j = 0; j < nodes[i]->output.dataCount; j++) nodes[i]->output.data[j].value = (float)FCos(nodes[i]->output.data[j].value);
                        } break;
                        case FNODE_SIN:
                        {
                            for (int j = 0; j < nodes[i]->output.dataCount; j++) nodes[i]->output.data[j].value = (float)FSin(nodes[i]->output.data[j].value);
                        } break;
                        case FNODE_TAN:
                        {
                            for (int j = 0; j < nodes[i]->output.dataCount; j++) nodes[i]->output.data[j].value = (float)FTan(nodes[i]->output.data[j].value);
                        } break;
                        case FNODE_DEG2RAD:
                        {
                            for (int j = 0; j < nodes[i]->output.dataCount; j++) nodes[i]->output.data[j].value *= DEG2RAD;
                        } break;
                        case FNODE_RAD2DEG:
                        {
                            for (int j = 0; j < nodes[i]->output.dataCount; j++) nodes[i]->output.data[j].value *= RAD2DEG;
                        } break;
                        case FNODE_NORMALIZE:
                        {
                            switch (nodes[i]->output.dataCount)
                            {
                                case 2:
                                {
                                    Vector2 temp = { nodes[i]->output.data[0].value, nodes[i]->output.data[1].value };
                                    temp = FVector2Normalize(temp);
                                    nodes[i]->output.data[0].value = temp.x;
                                    nodes[i]->output.data[1].value = temp.y;
                                } break;
                                case 3:
                                {
                                    Vector3 temp = { nodes[i]->output.data[0].value, nodes[i]->output.data[1].value, nodes[i]->output.data[2].value };
                                    temp = FVector3Normalize(temp);
                                    nodes[i]->output.data[0].value = temp.x;
                                    nodes[i]->output.data[1].value = temp.y;
                                    nodes[i]->output.data[2].value = temp.z;
                                } break;
                                case 4:
                                {
                                    Vector4 temp = { nodes[i]->output.data[0].value, nodes[i]->output.data[1].value, nodes[i]->output.data[2].value, nodes[i]->output.data[3].value };
                                    temp = FVector4Normalize(temp);
                                    nodes[i]->output.data[0].value = temp.x;
                                    nodes[i]->output.data[1].value = temp.y;
                                    nodes[i]->output.data[2].value = temp.z;
                                    nodes[i]->output.data[3].value = temp.w;
                                } break;
                                default: break;
                            }
                        } break;
                        case FNODE_NEGATE:
                        {
                            for (int j = 0; j < nodes[i]->output.dataCount; j++) nodes[i]->output.data[j].value *= -1;
                        } break;
                        case FNODE_RECIPROCAL:
                        {
                            for (int j = 0; j < nodes[i]->output.dataCount; j++) nodes[i]->output.data[j].value = 1/nodes[i]->output.data[j].value;
                        } break;
                        case FNODE_SQRT:
                        {
                            for (int j = 0; j < nodes[i]->output.dataCount; j++) nodes[i]->output.data[j].value = FSquareRoot(nodes[i]->output.data[j].value);
                        } break;
                        case FNODE_TRUNC:
                        {
                            for (int j = 0; j < nodes[i]->output.dataCount; j++) nodes[i]->output.data[j].value = FTrunc(nodes[i]->output.data[j].value);
                        } break;
                        case FNODE_ROUND:
                        {
                            for (int j = 0; j < nodes[i]->output.dataCount; j++) nodes[i]->output.data[j].value = FRound(nodes[i]->output.data[j].value);
                        } break;
                        case FNODE_CEIL:
                        {
                            for (int j = 0; j < nodes[i]->output.dataCount; j++) nodes[i]->output.data[j].value = FCeil(nodes[i]->output.data[j].value);
                        } break;
                        case FNODE_CLAMP01:
                        {
                            for (int j = 0; j < nodes[i]->output.dataCount; j++) nodes[i]->output.data[j].value = FClamp(nodes[i]->output.data[j].value, 0.0f, 1.0f);
                        } break;
                        case FNODE_EXP2:
                        {
                            for (int j = 0; j < nodes[i]->output.dataCount; j++) nodes[i]->output.data[j].value = FPower(2.0f, nodes[i]->output.data[j].value);
                        } break;
                        case FNODE_POWER:
                        {
                            if (nodes[i]->inputsCount == 2)
                            {
                                int expIndex = GetNodeIndex(nodes[i]->inputs[1]);

                                if (nodes[expIndex]->output.dataCount == 1)
                                {
                                    for (int j = 0; j < nodes[i]->output.dataCount; j++) nodes[i]->output.data[j].value = FPower(nodes[i]->output.data[j].value, nodes[expIndex]->output.data[0].value);
                                }
                                else TraceLogFNode(false, "values count of node %i should be 1 because it is an exponent (currently count: %i)", nodes[expIndex]->id, nodes[expIndex]->output.dataCount);
                            }
                            else
                            {
                                nodes[i]->output.dataCount = 0;
                                for (int j = 0; j < MAX_VALUES; j++) nodes[i]->output.data[j].value = 0.0f;
                            }
                        } break;
                        case FNODE_STEP:
                        {
                            if (nodes[i]->inputsCount == 2)
                            {
                                int indexB = GetNodeIndex(nodes[i]->inputs[1]);

                                if (nodes[indexB]->output.dataCount == 1) nodes[i]->output.data[0].value = ((nodes[i]->output.data[0].value <= nodes[indexB]->output.data[0].value) ? 1.0f : 0.0f);
                                else TraceLogFNode(false, "values count of node %i should be 1 because it is an exponent (currently count: %i)", nodes[indexB]->id, nodes[indexB]->output.dataCount);
                            }
                            else
                            {
                                nodes[i]->output.dataCount = 0;
                                for (int j = 0; j < MAX_VALUES; j++) nodes[i]->output.data[j].value = 0.0f;
                            }
                        } break;
                        case FNODE_POSTERIZE:
                        {
                            if (nodes[i]->inputsCount == 2)
                            {
                                int expIndex = GetNodeIndex(nodes[i]->inputs[1]);

                                if (nodes[expIndex]->output.dataCount == 1)
                                {
                                    for (int j = 0; j < nodes[i]->output.dataCount; j++) nodes[i]->output.data[j].value = FPosterize(nodes[i]->output.data[j].value, nodes[expIndex]->output.data[0].value);
                                }
                            }
                            else
                            {
                                nodes[i]->output.dataCount = 0;
                                for (int j = 0; j < MAX_VALUES; j++) nodes[i]->output.data[j].value = 0.0f;
                            }
                        } break;
                        case FNODE_MAX:
                        case FNODE_MIN:
                        {
                            for (int j = 1; j < nodes[i]->inputsCount; j++)
                            {
                                int inputIndex = GetNodeIndex(nodes[i]->inputs[j]);

                                for (int k = 0; k < nodes[i]->output.dataCount; k++)
                                {
                                    if ((nodes[inputIndex]->output.data[k].value > nodes[i]->output.data[k].value) && (nodes[i]->type == FNODE_MAX)) nodes[i]->output.data[k].value = nodes[inputIndex]->output.data[k].value;
                                    else if ((nodes[inputIndex]->output.data[k].value < nodes[i]->output.data[k].value) && (nodes[i]->type == FNODE_MIN)) nodes[i]->output.data[k].value = nodes[inputIndex]->output.data[k].value;
                                }
                            }
                        } break;
                        case FNODE_LERP:
                        {                                
                            if (nodes[i]->inputsCount == 3)
                            {
                                int indexA = GetNodeIndex(nodes[i]->inputs[0]);
                                int indexB = GetNodeIndex(nodes[i]->inputs[1]);
                                int indexC = GetNodeIndex(nodes[i]->inputs[2]);

                                for (int j = 0; j < nodes[i]->output.dataCount; j++) nodes[i]->output.data[j].value = FLerp(nodes[indexA]->output.data[j].value, nodes[indexB]->output.data[j].value, nodes[indexC]->output.data[j].value);
                            }
                            else
                            {
                                nodes[i]->output.dataCount = 0;
                                for (int j = 0; j < MAX_VALUES; j++) nodes[i]->output.data[j].value = 0.0f;
                            }
                        } break;
                        case FNODE_SMOOTHSTEP:
                        {                                
                            if (nodes[i]->inputsCount == 3)
                            {
                                int indexA = GetNodeIndex(nodes[i]->inputs[0]);
                                int indexB = GetNodeIndex(nodes[i]->inputs[1]);
                                int indexC = GetNodeIndex(nodes[i]->inputs[2]);

                                for (int j = 0; j < nodes[i]->output.dataCount; j++) nodes[i]->output.data[j].value = FSmoothStep(nodes[indexA]->output.data[j].value, nodes[indexB]->output.data[j].value, nodes[indexC]->output.data[j].value);
                            }
                            else
                            {
                                nodes[i]->output.dataCount = 0;
                                for (int j = 0; j < MAX_VALUES; j++) nodes[i]->output.data[j].value = 0.0f;
                            }
                        } break;
                        case FNODE_CROSSPRODUCT:
                        {
                            if (nodes[i]->inputsCount == 2)
                            {
                                int indexA = GetNodeIndex(nodes[i]->inputs[0]);
                                int indexB = GetNodeIndex(nodes[i]->inputs[1]);

                                for (int j = 0; j < MAX_VALUES; j++) nodes[i]->output.data[j].value = 0.0f;
                                
                                Vector3 vectorA = { nodes[indexA]->output.data[0].value, nodes[indexA]->output.data[1].value, nodes[indexA]->output.data[2].value };
                                Vector3 vectorB = { nodes[indexB]->output.data[0].value, nodes[indexB]->output.data[1].value, nodes[indexB]->output.data[2].value };
                                Vector3 cross = FCrossProduct(vectorA, vectorB);

                                nodes[i]->output.dataCount = 3;
                                nodes[i]->output.data[0].value = cross.x;
                                nodes[i]->output.data[1].value = cross.y;
                                nodes[i]->output.data[2].value = cross.z;
                            }
                            else
                            {
                                nodes[i]->output.dataCount = 0;
                                for (int j = 0; j < MAX_VALUES; j++) nodes[i]->output.data[j].value = 0.0f;
                            }
                        } break;
                        case FNODE_DESATURATE:
                        {
                            if (nodes[i]->inputsCount == 2)
                            {
                                int index = GetNodeIndex(nodes[i]->inputs[0]);
                                int index1 = GetNodeIndex(nodes[i]->inputs[1]);

                                for (int j = 0; j < MAX_VALUES; j++) nodes[i]->output.data[j].value = nodes[index]->output.data[j].value;
                                nodes[i]->output.dataCount = nodes[index]->output.dataCount;

                                float amount = FClamp(nodes[index1]->output.data[0].value, 0.0f, 1.0f);
                                float luminance = 0.3f*nodes[i]->output.data[0].value + 0.6f*nodes[i]->output.data[1].value + 0.1f*nodes[i]->output.data[2].value;

                                nodes[i]->output.data[0].value = nodes[i]->output.data[0].value + amount*(luminance - nodes[i]->output.data[0].value);
                                nodes[i]->output.data[1].value = nodes[i]->output.data[1].value + amount*(luminance - nodes[i]->output.data[1].value);
                                nodes[i]->output.data[2].value = nodes[i]->output.data[2].value + amount*(luminance - nodes[i]->output.data[2].value);
                            }
                            else
                            {
                                nodes[i]->output.dataCount = 0;
                                for (int j = 0; j < MAX_VALUES; j++) nodes[i]->output.data[j].value = 0.0f;
                            }
                        } break;
                        case FNODE_DISTANCE:
                        {
                            if (nodes[i]->inputsCount == 2)
                            {
                                int indexA = GetNodeIndex(nodes[i]->inputs[0]);
                                int indexB = GetNodeIndex(nodes[i]->inputs[1]);

                                switch (nodes[i]->output.dataCount)
                                {
                                    case 1: nodes[i]->output.data[0].value = nodes[indexB]->output.data[0].value - nodes[indexA]->output.data[0].value; break;
                                    case 2:
                                    {
                                        Vector2 direction = { 0, 0 };
                                        direction.x = nodes[indexB]->output.data[0].value - nodes[indexA]->output.data[0].value;
                                        direction.y = nodes[indexB]->output.data[1].value - nodes[indexA]->output.data[1].value;

                                        nodes[i]->output.data[0].value = FVector2Length(direction);
                                    } break;
                                    case 3:
                                    {
                                        Vector3 direction = { 0, 0, 0 };
                                        direction.x = nodes[indexB]->output.data[0].value - nodes[indexA]->output.data[0].value;
                                        direction.y = nodes[indexB]->output.data[1].value - nodes[indexA]->output.data[1].value;
                                        direction.z = nodes[indexB]->output.data[2].value - nodes[indexA]->output.data[2].value;

                                        nodes[i]->output.data[0].value = FVector3Length(direction);
                                    } break;
                                    case 4:
                                    {
                                        Vector4 direction = { 0, 0, 0, 0 };
                                        direction.x = nodes[indexB]->output.data[0].value - nodes[indexA]->output.data[0].value;
                                        direction.y = nodes[indexB]->output.data[1].value - nodes[indexA]->output.data[1].value;
                                        direction.z = nodes[indexB]->output.data[2].value - nodes[indexA]->output.data[2].value;
                                        direction.w = nodes[indexB]->output.data[3].value - nodes[indexA]->output.data[3].value;

                                        nodes[i]->output.data[0].value = FVector4Length(direction);
                                    } break;
                                    default: break;
                                }

                                for (int j = 1; j < MAX_VALUES; j++) nodes[i]->output.data[j].value = 0.0f;
                                nodes[i]->output.dataCount = 1;
                            }
                            else
                            {
                                for (int j = 0; j < MAX_VALUES; j++) nodes[i]->output.data[j].value = 0.0f;
                                nodes[i]->output.dataCount = 0;
                            }
                        } break;
                        case FNODE_DOTPRODUCT:
                        {
                            if (nodes[i]->inputsCount == 2)
                            {
                                int indexA = GetNodeIndex(nodes[i]->inputs[0]);
                                int indexB = GetNodeIndex(nodes[i]->inputs[1]);

                                switch (nodes[i]->output.dataCount)
                                {
                                    case 2:
                                    {
                                        Vector2 vectorA = { nodes[indexA]->output.data[0].value, nodes[indexA]->output.data[1].value };
                                        Vector2 vectorB = { nodes[indexB]->output.data[0].value, nodes[indexB]->output.data[1].value };
                                        nodes[i]->output.data[0].value = FVector2Dot(vectorA, vectorB);
                                    } break;
                                    case 3:
                                    {
                                        Vector3 vectorA = { nodes[indexA]->output.data[0].value, nodes[indexA]->output.data[1].value, nodes[indexA]->output.data[2].value };
                                        Vector3 vectorB = { nodes[indexB]->output.data[0].value, nodes[indexB]->output.data[1].value, nodes[indexB]->output.data[2].value };
                                        nodes[i]->output.data[0].value = FVector3Dot(vectorA, vectorB);
                                    } break;
                                    case 4:
                                    {
                                        Vector4 vectorA = { nodes[indexA]->output.data[0].value, nodes[indexA]->output.data[1].value, nodes[indexA]->output.data[2].value, nodes[indexA]->output.data[3].value };
                                        Vector4 vectorB = { nodes[indexB]->output.data[0].value, nodes[indexB]->output.data[1].value, nodes[indexB]->output.data[2].value, nodes[indexB]->output.data[3].value };
                                        nodes[i]->output.data[0].value = FVector4Dot(vectorA, vectorB);
                                    } break;
                                    default: break;
                                }

                                for (int j = 1; j < MAX_VALUES; j++) nodes[i]->output.data[j].value = 0.0f;
                                nodes[i]->output.dataCount = 1;
                            }
                            else
                            {
                                for (int j = 0; j < MAX_VALUES; j++) nodes[i]->output.data[j].value = 0.0f;
                                nodes[i]->output.dataCount = 0;
                            }
                        } break;
                        case FNODE_LENGTH:
                        {
                            switch (nodes[i]->output.dataCount)
                            {
                                case 2: nodes[i]->output.data[0].value = FVector2Length((Vector2){ nodes[i]->output.data[0].value, nodes[i]->output.data[1].value }); break;
                                case 3: nodes[i]->output.data[0].value = FVector3Length((Vector3){ nodes[i]->output.data[0].value, nodes[i]->output.data[1].value, nodes[i]->output.data[2].value }); break;
                                case 4: nodes[i]->output.data[0].value = FVector4Length((Vector4){ nodes[i]->output.data[0].value, nodes[i]->output.data[1].value, nodes[i]->output.data[2].value, nodes[i]->output.data[3].value }); break;
                                default: break;
                            }

                            for (int j = 1; j < MAX_VALUES; j++) nodes[i]->output.data[j].value = 0.0f;
                            nodes[i]->output.dataCount = 1;
                        } break;
                        case FNODE_MULTIPLYMATRIX:
                        {
                            if (nodes[i]->inputsCount == 2)
                            {
                                int index = GetNodeIndex(nodes[i]->inputs[1]);

                                Matrix matrixA = { nodes[i]->output.data[0].value, nodes[i]->output.data[1].value, nodes[i]->output.data[2].value, nodes[i]->output.data[3].value, 
                                nodes[i]->output.data[4].value, nodes[i]->output.data[5].value, nodes[i]->output.data[6].value, nodes[i]->output.data[7].value, 
                                nodes[i]->output.data[8].value, nodes[i]->output.data[9].value, nodes[i]->output.data[10].value, nodes[i]->output.data[11].value, 
                                nodes[i]->output.data[12].value, nodes[i]->output.data[13].value, nodes[i]->output.data[14].value, nodes[i]->output.data[15].value, };
                                
                                Matrix matrixB = { nodes[index]->output.data[0].value, nodes[index]->output.data[1].value, nodes[index]->output.data[2].value, nodes[index]->output.data[3].value, 
                                nodes[index]->output.data[4].value, nodes[index]->output.data[5].value, nodes[index]->output.data[6].value, nodes[index]->output.data[7].value, 
                                nodes[index]->output.data[8].value, nodes[index]->output.data[9].value, nodes[index]->output.data[10].value, nodes[index]->output.data[11].value, 
                                nodes[index]->output.data[12].value, nodes[index]->output.data[13].value, nodes[index]->output.data[14].value, nodes[index]->output.data[15].value, };
                                
                                Matrix matrixAB = FMatrixMultiply(matrixA, matrixB);
                                
                                nodes[i]->output.data[0].value = matrixAB.m0;
                                nodes[i]->output.data[1].value = matrixAB.m1;
                                nodes[i]->output.data[2].value = matrixAB.m2;
                                nodes[i]->output.data[3].value = matrixAB.m3;
                                nodes[i]->output.data[4].value = matrixAB.m4;
                                nodes[i]->output.data[5].value = matrixAB.m5;
                                nodes[i]->output.data[6].value = matrixAB.m6;
                                nodes[i]->output.data[7].value = matrixAB.m7;
                                nodes[i]->output.data[8].value = matrixAB.m8;
                                nodes[i]->output.data[9].value = matrixAB.m9;
                                nodes[i]->output.data[10].value = matrixAB.m10;
                                nodes[i]->output.data[11].value = matrixAB.m11;
                                nodes[i]->output.data[12].value = matrixAB.m12;
                                nodes[i]->output.data[13].value = matrixAB.m13;
                                nodes[i]->output.data[14].value = matrixAB.m14;
                                nodes[i]->output.data[15].value = matrixAB.m15;
                            }
                        } break;
                        case FNODE_TRANSPOSE:
                        {
                            Matrix matrix = { nodes[i]->output.data[0].value, nodes[i]->output.data[1].value, nodes[i]->output.data[2].value, nodes[i]->output.data[3].value, 
                            nodes[i]->output.data[4].value, nodes[i]->output.data[5].value, nodes[i]->output.data[6].value, nodes[i]->output.data[7].value, 
                            nodes[i]->output.data[8].value, nodes[i]->output.data[9].value, nodes[i]->output.data[10].value, nodes[i]->output.data[11].value, 
                            nodes[i]->output.data[12].value, nodes[i]->output.data[13].value, nodes[i]->output.data[14].value, nodes[i]->output.data[15].value, };

                            FMatrixTranspose(&matrix);

                            nodes[i]->output.data[0].value = matrix.m0;
                            nodes[i]->output.data[1].value = matrix.m1;
                            nodes[i]->output.data[2].value = matrix.m2;
                            nodes[i]->output.data[3].value = matrix.m3;
                            nodes[i]->output.data[4].value = matrix.m4;
                            nodes[i]->output.data[5].value = matrix.m5;
                            nodes[i]->output.data[6].value = matrix.m6;
                            nodes[i]->output.data[7].value = matrix.m7;
                            nodes[i]->output.data[8].value = matrix.m8;
                            nodes[i]->output.data[9].value = matrix.m9;
                            nodes[i]->output.data[10].value = matrix.m10;
                            nodes[i]->output.data[11].value = matrix.m11;
                            nodes[i]->output.data[12].value = matrix.m12;
                            nodes[i]->output.data[13].value = matrix.m13;
                            nodes[i]->output.data[14].value = matrix.m14;
                            nodes[i]->output.data[15].value = matrix.m15;
                        } break;
                        case FNODE_PROJECTION:
                        case FNODE_REJECTION:
                        {
                            if (nodes[i]->inputsCount == 2)
                            {
                                int indexA = GetNodeIndex(nodes[i]->inputs[0]);
                                int indexB = GetNodeIndex(nodes[i]->inputs[1]);

                                switch (nodes[i]->output.dataCount)
                                {
                                    case 2:
                                    {
                                        Vector2 vectorA = { nodes[indexA]->output.data[0].value, nodes[indexA]->output.data[1].value };
                                        Vector2 vectorB = { nodes[indexB]->output.data[0].value, nodes[indexB]->output.data[1].value };
                                        Vector2 newVector = ((nodes[i]->type == FNODE_PROJECTION) ? FVector2Projection(vectorA, vectorB) : FVector2Rejection(vectorA, vectorB));

                                        nodes[i]->output.data[0].value = newVector.x;
                                        nodes[i]->output.data[1].value = newVector.y;

                                        for (int j = 2; j < MAX_VALUES; j++) nodes[i]->output.data[j].value = 0.0f;
                                        nodes[i]->output.dataCount = 2;
                                    } break;
                                    case 3:
                                    {
                                        Vector3 vectorA = { nodes[indexA]->output.data[0].value, nodes[indexA]->output.data[1].value, nodes[indexA]->output.data[2].value };
                                        Vector3 vectorB = { nodes[indexB]->output.data[0].value, nodes[indexB]->output.data[1].value, nodes[indexB]->output.data[2].value };
                                        Vector3 newVector = ((nodes[i]->type == FNODE_PROJECTION) ? FVector3Projection(vectorA, vectorB) : FVector3Rejection(vectorA, vectorB));

                                        nodes[i]->output.data[0].value = newVector.x;
                                        nodes[i]->output.data[1].value = newVector.y;
                                        nodes[i]->output.data[2].value = newVector.z;

                                        for (int j = 3; j < MAX_VALUES; j++) nodes[i]->output.data[j].value = 0.0f;
                                        nodes[i]->output.dataCount = 3;
                                    } break;
                                    case 4:
                                    {
                                        Vector4 vectorA = { nodes[indexA]->output.data[0].value, nodes[indexA]->output.data[1].value, nodes[indexA]->output.data[2].value, nodes[indexA]->output.data[3].value };
                                        Vector4 vectorB = { nodes[indexB]->output.data[0].value, nodes[indexB]->output.data[1].value, nodes[indexB]->output.data[2].value, nodes[indexB]->output.data[3].value };
                                        Vector4 newVector = ((nodes[i]->type == FNODE_PROJECTION) ? FVector4Projection(vectorA, vectorB) : FVector4Rejection(vectorA, vectorB));

                                        nodes[i]->output.data[0].value = newVector.x;
                                        nodes[i]->output.data[1].value = newVector.y;
                                        nodes[i]->output.data[2].value = newVector.z;
                                        nodes[i]->output.data[3].value = newVector.w;

                                        for (int j = 4; j < MAX_VALUES; j++) nodes[i]->output.data[j].value = 0.0f;
                                        nodes[i]->output.dataCount = 4;
                                    } break;
                                    default: break;
                                }
                            }
                            else
                            {
                                for (int j = 0; j < MAX_VALUES; j++) nodes[i]->output.data[j].value = 0.0f;
                                nodes[i]->output.dataCount = 0;
                            }
                        } break;
                        case FNODE_HALFDIRECTION:
                        {
                            if (nodes[i]->inputsCount == 2)
                            {
                                int indexA = GetNodeIndex(nodes[i]->inputs[0]);
                                int indexB = GetNodeIndex(nodes[i]->inputs[1]);
                                
                                bool check = true;
                                check = ((nodes[indexA]->type <= FNODE_E) || (nodes[indexA]->type >= FNODE_MATRIX));
                                if (check) check = ((nodes[indexB]->type <= FNODE_E) || (nodes[indexB]->type >= FNODE_MATRIX));

                                if (check)
                                {
                                    switch (nodes[i]->output.dataCount)
                                    {
                                        case 2:
                                        {
                                            Vector2 a = { nodes[indexA]->output.data[0].value, nodes[indexA]->output.data[1].value };
                                            Vector2 b = { nodes[indexB]->output.data[0].value, nodes[indexB]->output.data[1].value };
                                            Vector2 vectorA = FVector2Normalize(a);
                                            Vector2 vectorB = FVector2Normalize(b);
                                            Vector2 output = { vectorA.x + vectorB.x, vectorA.y + vectorB.y };

                                            nodes[i]->output.data[0].value = output.x;
                                            nodes[i]->output.data[1].value = output.y;

                                            for (int j = 2; j < MAX_VALUES; j++) nodes[i]->output.data[j].value = 0.0f;
                                            nodes[i]->output.dataCount = 2;
                                        } break;
                                        case 3:
                                        {
                                            Vector3 a = { nodes[indexA]->output.data[0].value, nodes[indexA]->output.data[1].value, nodes[indexA]->output.data[2].value };
                                            Vector3 b = { nodes[indexB]->output.data[0].value, nodes[indexB]->output.data[1].value, nodes[indexB]->output.data[2].value };
                                            Vector3 vectorA = FVector3Normalize(a);
                                            Vector3 vectorB = FVector3Normalize(b);
                                            Vector3 output = { vectorA.x + vectorB.x, vectorA.y + vectorB.y, vectorA.z + vectorB.z };

                                            nodes[i]->output.data[0].value = output.x;
                                            nodes[i]->output.data[1].value = output.y;
                                            nodes[i]->output.data[2].value = output.z;

                                            for (int j = 3; j < MAX_VALUES; j++) nodes[i]->output.data[j].value = 0.0f;
                                            nodes[i]->output.dataCount = 3;
                                        } break;
                                        case 4:
                                        {
                                            Vector4 a = { nodes[indexA]->output.data[0].value, nodes[indexA]->output.data[1].value, nodes[indexA]->output.data[2].value, nodes[indexA]->output.data[3].value };
                                            Vector4 b = { nodes[indexB]->output.data[0].value, nodes[indexB]->output.data[1].value, nodes[indexB]->output.data[2].value, nodes[indexB]->output.data[3].value };
                                            Vector4 vectorA = FVector4Normalize(a);
                                            Vector4 vectorB = FVector4Normalize(b);
                                            Vector4 output = { vectorA.x + vectorB.x, vectorA.y + vectorB.y, vectorA.z + vectorB.z, vectorA.w + vectorB.w };

                                            nodes[i]->output.data[0].value = output.x;
                                            nodes[i]->output.data[1].value = output.y;
                                            nodes[i]->output.data[2].value = output.z;
                                            nodes[i]->output.data[3].value = output.w;

                                            for (int j = 4; j < MAX_VALUES; j++) nodes[i]->output.data[j].value = 0.0f;
                                            nodes[i]->output.dataCount = 4;
                                        } break;
                                        default: break;
                                    }
                                }
                                else
                                {
                                    for (int j = 0; j < MAX_VALUES; j++) nodes[i]->output.data[j].value = 0.0f;
                                }
                            }
                            else
                            {
                                for (int j = 0; j < MAX_VALUES; j++) nodes[i]->output.data[j].value = 0.0f;
                                nodes[i]->output.dataCount = 0;
                            }
                        }
                        default: break;
                    }
                }
                else
                {
                    for (int k = 0; k < MAX_VALUES; k++) nodes[i]->output.data[k].value = 0.0f;
                    nodes[i]->output.dataCount = 0;
                }
            }

            if (nodes[i]->type > FNODE_VECTOR4)
            {
                for (int k = 0; k < nodes[i]->output.dataCount; k++) FFloatToString(nodes[i]->output.data[k].valueText, nodes[i]->output.data[k].value);
            }

            UpdateNodeShapes(nodes[i]);
        }
        else TraceLogFNode(true, "error trying to calculate values for a null referenced node");
    }
}

// Compiles all node structure to create the GLSL fragment shader in output folder
void CompileShader()
{
    if (loadedShader) UnloadShader(shader);
    remove(DATA_PATH);
    remove(VERTEX_PATH);
    remove(FRAGMENT_PATH);

    FILE *dataFile = fopen(DATA_PATH, "w");
    if (dataFile != NULL)
    {
        int count = 0;
        for (int i = 0; i < MAX_NODES; i++)
        {
            for (int k = 0; k < nodesCount; k++)
            {
                if (nodes[k]->id == i)
                {
                    float type = (float)nodes[k]->type;
                    float inputs[MAX_INPUTS] = { (float)nodes[k]->inputs[0], (float)nodes[k]->inputs[1], (float)nodes[k]->inputs[2], (float)nodes[k]->inputs[3] };
                    float inputsCount = (float)nodes[k]->inputsCount;
                    float inputsLimit = (float)nodes[k]->inputsLimit;
                    float dataCount = (float)nodes[k]->output.dataCount;
                    float data[MAX_VALUES] = { nodes[k]->output.data[0].value, nodes[k]->output.data[1].value, nodes[k]->output.data[2].value, nodes[k]->output.data[3].value, nodes[k]->output.data[4].value,
                    nodes[k]->output.data[5].value, nodes[k]->output.data[6].value, nodes[k]->output.data[7].value, nodes[k]->output.data[8].value, nodes[k]->output.data[9].value, nodes[k]->output.data[10].value,
                    nodes[k]->output.data[11].value, nodes[k]->output.data[12].value, nodes[k]->output.data[13].value, nodes[k]->output.data[14].value, nodes[k]->output.data[15].value };
                    float shapeX = (float)nodes[k]->shape.x;
                    float shapeY = (float)nodes[k]->shape.y;
                    
                    fprintf(dataFile, "%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,\n", type,
                    inputs[0], inputs[1], inputs[2], inputs[3], inputsCount, inputsLimit, dataCount, data[0], data[1], data[2],
                    data[3], data[4], data[5], data[6], data[7], data[8], data[9], data[10], data[11], data[12], data[13], data[14],
                    data[15], shapeX, shapeY);
                    
                    count++;
                    break;
                }
            }
            
            if (count == nodesCount) break;
        }
        
        count = 0;
        
        for (int i = 0; i < MAX_NODES; i++)
        {
            for (int k = 0; k < nodesCount; k++)
            {
                if (lines[k]->id == i)
                {
                    fprintf(dataFile, "?%i?%i\n", lines[k]->from, lines[k]->to);
                    
                    count++;
                    break;
                }
            }
            
            if (count == linesCount) break;
        }

        fclose(dataFile);
    }
    else TraceLogFNode(true, "error when trying to open and write in data file");
    
    FILE *vertexFile = fopen(VERTEX_PATH, "w");
    if (vertexFile != NULL)
    {
        // Vertex shader definition to embed, no external file required
        const char vHeader[] = 
        "#version 330                     \n\n";
        fprintf(vertexFile, vHeader);

        const char vIn[] = 
        "in vec3 vertexPosition;            \n"
        "in vec3 vertexNormal;              \n"
        "in vec2 vertexTexCoord;            \n"
        "in vec4 vertexColor;             \n\n";
        fprintf(vertexFile, vIn);

        const char vOut[] = 
        "out vec3 fragPosition;             \n"
        "out vec3 fragNormal;               \n"
        "out vec2 fragTexCoord;             \n"
        "out vec4 fragColor;              \n\n";
        fprintf(vertexFile, vOut);

        const char vUniforms[] = 
        "uniform mat4 mvpMatrix;          \n\n";
        fprintf(vertexFile, vUniforms);

        const char vMain[] = 
        "void main()                        \n"
        "{                                  \n"
        "    fragPosition = vertexPosition; \n"
        "    fragNormal = vertexNormal;     \n"
        "    fragTexCoord = vertexTexCoord; \n"
        "    fragColor = vertexColor;     \n\n";
        fprintf(vertexFile, vMain);

        int index = GetNodeIndex(nodes[0]->inputs[0]);
        CompileNode(nodes[index], vertexFile, false);

        switch (nodes[index]->output.dataCount)
        {
            case 1: fprintf(vertexFile, "\n    gl_Position = vec4(node_%02i, node_%02i, node_%02i, 1.0);\n}", nodes[0]->inputs[0], nodes[0]->inputs[0], nodes[0]->inputs[0]); break;
            case 2: fprintf(vertexFile, "\n    gl_Position = vec4(node_%02i.xy, 0.0, 1.0);\n}", nodes[0]->inputs[0]); break;
            case 3: fprintf(vertexFile, "\n    gl_Position = vec4(node_%02i.xyz, 1.0);\n}", nodes[0]->inputs[0]); break;
            case 4: fprintf(vertexFile, "\n    gl_Position = node_%02i;\n}", nodes[0]->inputs[0]); break;
            case 16: fprintf(vertexFile, "\n    gl_Position = node_%02i;\n}", nodes[0]->inputs[0]); break;
            default: break;
        }

        fclose(vertexFile);
    }
    else TraceLogFNode(true, "error when trying to open and write in vertex shader file");

    FILE *fragmentFile = fopen(FRAGMENT_PATH, "w");
    if (fragmentFile != NULL)
    {
        // Fragment shader definition to embed, no external file required
        const char fHeader[] = 
        "#version 330                     \n\n";
        fprintf(fragmentFile, fHeader);

        fprintf(fragmentFile, "// Input attributes\n");
        const char fIn[] = 
        "in vec3 fragPosition;             \n"
        "in vec3 fragNormal;               \n"
        "in vec2 fragTexCoord;             \n"
        "in vec4 fragColor;              \n\n";
        fprintf(fragmentFile, fIn);

        fprintf(fragmentFile, "// Uniform attributes\n");
        const char fUniforms[] = 
        "uniform sampler2D texture0;       \n"
        "uniform vec4 colDiffuse;          \n"
        "uniform vec3 viewDirection;       \n"
        "uniform mat4 modelMatrix;       \n\n";
        fprintf(fragmentFile, fUniforms);

        fprintf(fragmentFile, "// Output attributes\n");
        const char fOut[] = 
        "out vec4 finalColor;            \n\n";
        fprintf(fragmentFile, fOut);
        
        fprintf(fragmentFile, "// Constant values\n");
        for (int i = 0; i < nodesCount; i++)
        {
            switch (nodes[i]->type)
            {
                case FNODE_PI: fprintf(fragmentFile, "const float node_%02i = 3.14159265358979323846;\n"); break;
                case FNODE_E: fprintf(fragmentFile, "const float node_%02i = 2.71828182845904523536;\n"); break;
                case FNODE_VALUE:
                {
                    const char fConstantValue[] = "const float node_%02i = %.3f;\n";
                    fprintf(fragmentFile, fConstantValue, nodes[i]->id, nodes[i]->output.data[0].value);
                } break;
                case FNODE_VECTOR2:
                {
                    const char fConstantVector2[] = "const vec2 node_%02i = vec2(%.3f, %.3f);\n";
                    fprintf(fragmentFile, fConstantVector2, nodes[i]->id, nodes[i]->output.data[0].value, nodes[i]->output.data[1].value);
                } break;
                case FNODE_VECTOR3:
                {
                    const char fConstantVector3[] = "const vec3 node_%02i = vec3(%.3f, %.3f, %.3f);\n";
                    fprintf(fragmentFile, fConstantVector3, nodes[i]->id, nodes[i]->output.data[0].value, nodes[i]->output.data[1].value, nodes[i]->output.data[2].value);
                } break;
                case FNODE_VECTOR4:
                {
                    const char fConstantVector4[] = "const vec4 node_%02i = vec4(%.3f, %.3f, %.3f, %.3f);\n";
                    fprintf(fragmentFile, fConstantVector4, nodes[i]->id, nodes[i]->output.data[0].value, nodes[i]->output.data[1].value, nodes[i]->output.data[2].value, nodes[i]->output.data[3].value);
                } break;
                default: break;
            }
        }

        const char fMain[] = 
        "\n"
        "void main()                        \n"
        "{                                  \n";
        fprintf(fragmentFile, fMain);

        int index = GetNodeIndex(nodes[1]->inputs[0]);
        CompileNode(nodes[index], fragmentFile, true);

        switch (nodes[index]->output.dataCount)
        {
            case 1: fprintf(fragmentFile, "\n    finalColor = vec4(node_%02i, node_%02i, node_%02i, 1.0);\n}", nodes[1]->inputs[0], nodes[1]->inputs[0], nodes[1]->inputs[0]); break;
            case 2: fprintf(fragmentFile, "\n    finalColor = vec4(node_%02i.xy, 0.0, 1.0);\n}", nodes[1]->inputs[0]); break;
            case 3: fprintf(fragmentFile, "\n    finalColor = vec4(node_%02i.xyz, 1.0);\n}", nodes[1]->inputs[0]); break;
            case 4: fprintf(fragmentFile, "\n    finalColor = node_%02i;\n}", nodes[1]->inputs[0]); break;
            default: break;
        }

        fclose(fragmentFile);
    }
    else TraceLogFNode(true, "error when trying to open and write in vertex shader file");

    shader = LoadShader(VERTEX_PATH, FRAGMENT_PATH);
    if (shader.id != 0)
    {
        loadedShader = true;
        model.material.shader = shader;
        viewUniform = GetShaderLocation(shader, "viewDirection");
        transformUniform = GetShaderLocation(shader, "modelMatrix");
    }
}

// Compiles a specific node checking its inputs and writing current node operation in shader
void CompileNode(FNode node, FILE *file, bool fragment)
{
    // Check if current node is an operator
    if ((node->inputsCount > 0) || ((node->type < FNODE_MATRIX) && (node->type > FNODE_E)))
    {
        // Check for operator nodes in inputs to compile them first
        for (int i = 0; i < node->inputsCount; i++)
        {
            int index = GetNodeIndex(node->inputs[i]);
            if (nodes[index]->type > FNODE_VECTOR4 || ((nodes[index]->type < FNODE_MATRIX) && (nodes[index]->type > FNODE_E))) CompileNode(nodes[index], file, fragment);
        }

        // Store variable declaration into a string
        char check[16] = { '\0' };
        switch (node->output.dataCount)
        {
            case 1: sprintf(check, "float node_%02i", node->id); break;
            case 2: sprintf(check, "vec2 node_%02i", node->id); break;
            case 3:
            {
                if (fragment) sprintf(check, "vec3 node_%02i", node->id);
                else if (node->type == FNODE_VERTEXPOSITION) sprintf(check, "vec4 node_%02i", node->id); 
            } break;
            case 4: sprintf(check, "vec4 node_%02i", node->id); break;
            case 16: sprintf(check, "mat4 node_%02i", node->id); break;
            default: break;
        }

        // Check if current node is previously defined and declared
        if (!FSearch(FRAGMENT_PATH, check))
        {
            // Variable definition based on current node output data count
            char body[1024] = { '\0' };
            char definition[32] = { '\0' };
            switch (node->output.dataCount)
            {
                case 1: sprintf(definition, "    float node_%02i = ", node->id); break;
                case 2: sprintf(definition, "    vec2 node_%02i = ", node->id); break;
                case 3:
                {
                    if (fragment) sprintf(definition, "    vec3 node_%02i = ", node->id);
                    else if (node->type == FNODE_VERTEXPOSITION) sprintf(definition, "    vec4 node_%02i = ", node->id);
                } break;
                case 4: sprintf(definition, "    vec4 node_%02i = ", node->id); break;
                case 16: sprintf(definition, "    mat4 node_%02i = ", node->id); break;
                default: break;
            }
            strcat(body, definition);

            if ((node->type < FNODE_MATRIX) && (node->type > FNODE_E))
            {
                char temp[32] = { '\0' };
                switch (node->type)
                {
                    case FNODE_VERTEXPOSITION:
                    {
                        if (fragment) strcat(body, "fragPosition;\n");
                        else strcat(body, "vec4(vertexPosition, 1.0);\n");
                    } break;
                    case FNODE_VERTEXNORMAL: strcat(body, "fragNormal;\n"); break;
                    case FNODE_FRESNEL: strcat(body, "1 - dot(fragNormal, viewDirection);\n"); break;
                    case FNODE_VIEWDIRECTION: strcat(body, "viewDirection;\n"); break;
                    case FNODE_MVP: strcat(body, "mvpMatrix;\n"); break;
                    default: break;
                }
            }
            else if ((node->type >= FNODE_ADD && node->type <= FNODE_DIVIDE) || node->type == FNODE_MULTIPLYMATRIX)
            {
                // Operate with each input node
                for (int i = 0; i < node->inputsCount; i++)
                {
                    char temp[32] = { '\0' };
                    if ((i+1) == node->inputsCount) sprintf(temp, "node_%02i;\n", node->inputs[i]);
                    else
                    {
                        sprintf(temp, "node_%02i", node->inputs[i]);
                        switch (node->type)
                        {
                            case FNODE_ADD: strcat(temp, " + "); break;
                            case FNODE_SUBTRACT: strcat(temp, " - "); break;
                            case FNODE_MULTIPLYMATRIX:
                            case FNODE_MULTIPLY: strcat(temp, "*"); break;
                            case FNODE_DIVIDE: strcat(temp, "/"); break;
                            default: break;
                        }
                    }

                    strcat(body, temp);
                }
            }
            else if (node->type >= FNODE_APPEND)
            {
                char temp[32] = { '\0' };
                switch (node->type)
                {
                    case FNODE_APPEND:
                    {
                        switch (node->output.dataCount)
                        {
                            case 1: sprintf(temp, "node_%02i;\n", node->inputs[0]); break;
                            case 2: sprintf(temp, "vec2(node_%02i, node_%02i);\n", node->inputs[0], node->inputs[1]); break;
                            case 3: sprintf(temp, "vec3(node_%02i, node_%02i, node_%02i);\n", node->inputs[0], node->inputs[1], node->inputs[2]); break;
                            case 4: sprintf(temp, "vec4(node_%02i, node_%02i, node_%02i, node_%02i);\n", node->inputs[0], node->inputs[1], node->inputs[2], node->inputs[3]); break;
                            default: break;
                        }
                    } break;
                    case FNODE_ONEMINUS: sprintf(temp, "(1 - node_%02i);\n", node->inputs[0]); break;
                    case FNODE_ABS: sprintf(temp, "abs(node_%02i);\n", node->inputs[0]); break;
                    case FNODE_COS: sprintf(temp, "cos(node_%02i);\n", node->inputs[0]); break;
                    case FNODE_SIN: sprintf(temp, "sin(node_%02i);\n", node->inputs[0]); break;
                    case FNODE_TAN: sprintf(temp, "tan(node_%02i);\n", node->inputs[0]); break;
                    case FNODE_DEG2RAD: sprintf(temp, "node_%02i*(3.14159265358979323846/180.0);\n", node->inputs[0]); break;
                    case FNODE_RAD2DEG: sprintf(temp, "node_%02i*(180.0/3.14159265358979323846);\n", node->inputs[0]); break;
                    case FNODE_NORMALIZE: sprintf(temp, "normalize(node_%02i);\n", node->inputs[0]); break;
                    case FNODE_NEGATE: sprintf(temp, "node_%02i*-1;\n", node->inputs[0]); break;
                    case FNODE_RECIPROCAL: sprintf(temp, "1/node_%02i;\n", node->inputs[0]); break;
                    case FNODE_SQRT: sprintf(temp, "sqrt(node_%02i);\n", node->inputs[0]); break;
                    case FNODE_TRUNC: sprintf(temp, "trunc(node_%02i);\n", node->inputs[0]); break;
                    case FNODE_ROUND: sprintf(temp, "round(node_%02i);\n", node->inputs[0]); break;
                    case FNODE_CEIL: sprintf(temp, "ceil(node_%02i);\n", node->inputs[0]); break;
                    case FNODE_CLAMP01: sprintf(temp, "clamp(node_%02i, 0.0, 1.0);\n", node->inputs[0]); break;
                    case FNODE_EXP2: sprintf(temp, "exp2(node_%02i);\n", node->inputs[0]); break;
                    case FNODE_POWER: sprintf(temp, "pow(node_%02i, node_%02i);\n", node->inputs[0], node->inputs[1]); break;
                    case FNODE_STEP: sprintf(temp, "((node_%02i <= node_%02i) ? 1.0 : 0.0);\n", node->inputs[0], node->inputs[1]); break;
                    case FNODE_POSTERIZE: sprintf(temp, "floor(node_%02i*node_%02i)/node_%02i;\n", node->inputs[0], node->inputs[1], node->inputs[1]); break;
                    case FNODE_MAX: sprintf(temp, "max(node_%02i, node_%02i);\n", node->inputs[0], node->inputs[1], node->inputs[1]); break;
                    case FNODE_MIN: sprintf(temp, "min(node_%02i, node_%02i);\n", node->inputs[0], node->inputs[1], node->inputs[1]); break;
                    case FNODE_LERP: sprintf(temp, "lerp(node_%02i, node_%02i, node_%02i);\n", node->inputs[0], node->inputs[1], node->inputs[2]); break;
                    case FNODE_SMOOTHSTEP: sprintf(temp, "smoothstep(node_%02i, node_%02i, node_%02i);\n", node->inputs[0], node->inputs[1], node->inputs[2]); break;
                    case FNODE_CROSSPRODUCT: sprintf(temp, "cross(node_%02i, node_%02i);\n", node->inputs[0], node->inputs[1]); break;
                    case FNODE_DESATURATE:
                    {
                        switch (node->output.dataCount)
                        {
                            case 1: sprintf(temp, "mix(node_%02i, 0.3, node_%02i);\n", node->inputs[0], node->inputs[1]); break;
                            case 2: sprintf(temp, "vec2(mix(node_%02i.x, vec2(0.3, 0.59), node_%02i));\n", node->inputs[0], node->inputs[1]); break;
                            case 3: sprintf(temp, "vec3(mix(node_%02i.xyz, vec3(0.3, 0.59, 0.11), node_%02i));\n", node->inputs[0], node->inputs[1]); break;
                            case 4: sprintf(temp, "vec4(mix(node_%02i.xyz, vec3(0.3, 0.59, 0.11), node_%02i), 1.0);\n", node->inputs[0], node->inputs[1]); break;
                            default: break;
                        }
                    } break;
                    case FNODE_DISTANCE: sprintf(temp, "distance(node_%02i, node_%02i);\n", node->inputs[0], node->inputs[1]); break;
                    case FNODE_DOTPRODUCT: sprintf(temp, "dot(node_%02i, node_%02i);\n", node->inputs[0], node->inputs[1]); break;
                    case FNODE_LENGTH: sprintf(temp, "length(node_%02i);\n", node->inputs[0]); break;
                    case FNODE_TRANSPOSE: sprintf(temp, "transpose(node_%02i);\n", node->inputs[0]); break;
                    case FNODE_PROJECTION:
                    {
                        switch (node->output.dataCount)
                        {
                            case 2: sprintf(temp, "vec2(dot(node_%02i, node_%02i)/dot(node_%02i, node_%02i)*node_%02i.x, dot(node_%02i, node_%02i)/dot(node_%02i, node_%02i)*node_%02i.y);\n", 
                            node->inputs[0], node->inputs[1], node->inputs[1], node->inputs[1], node->inputs[1], node->inputs[0], node->inputs[1], node->inputs[1], node->inputs[1], node->inputs[1]); break;
                            case 3: sprintf(temp, "vec3(dot(node_%02i, node_%02i)/dot(node_%02i, node_%02i)*node_%02i.x, dot(node_%02i, node_%02i)/dot(node_%02i, node_%02i)*node_%02i.y, dot(node_%02i, node_%02i)/dot(node_%02i, node_%02i)*node_%02i.z);\n", 
                            node->inputs[0], node->inputs[1], node->inputs[1], node->inputs[1], node->inputs[1], node->inputs[0], node->inputs[1], node->inputs[1], node->inputs[1], node->inputs[1], node->inputs[0], node->inputs[1], node->inputs[1], node->inputs[1], node->inputs[1]); break;
                            case 4: sprintf(temp, "vec4(dot(node_%02i, node_%02i)/dot(node_%02i, node_%02i)*node_%02i.x, dot(node_%02i, node_%02i)/dot(node_%02i, node_%02i)*node_%02i.y, dot(node_%02i, node_%02i)/dot(node_%02i, node_%02i)*node_%02i.z, dot(node_%02i, node_%02i)/dot(node_%02i, node_%02i)*node_%02i.w);\n", 
                            node->inputs[0], node->inputs[1], node->inputs[1], node->inputs[1], node->inputs[1], node->inputs[0], node->inputs[1], node->inputs[1], node->inputs[1], node->inputs[1], node->inputs[0], node->inputs[1], node->inputs[1], node->inputs[1], node->inputs[1], node->inputs[0], node->inputs[1], node->inputs[1], node->inputs[1], node->inputs[1]); break;
                            default: break;
                        }
                    }
                    case FNODE_REJECTION:
                    {
                        switch (node->output.dataCount)
                        {
                            case 2: sprintf(temp, "vec2(node_%02i.x - dot(node_%02i, node_%02i)/dot(node_%02i, node_%02i)*node_%02i.x, node_%02i.y - dot(node_%02i, node_%02i)/dot(node_%02i, node_%02i)*node_%02i.y);\n", 
                            node->inputs[0], node->inputs[0], node->inputs[1], node->inputs[1], node->inputs[1], node->inputs[1], node->inputs[0], node->inputs[0], node->inputs[1], node->inputs[1], node->inputs[1], node->inputs[1]); break;
                            case 3: sprintf(temp, "vec3(node_%02i.x - dot(node_%02i, node_%02i)/dot(node_%02i, node_%02i)*node_%02i.x, node_%02i.y - dot(node_%02i, node_%02i)/dot(node_%02i, node_%02i)*node_%02i.y, node_%02i.z - dot(node_%02i, node_%02i)/dot(node_%02i, node_%02i)*node_%02i.z);\n", 
                            node->inputs[0], node->inputs[0], node->inputs[1], node->inputs[1], node->inputs[1], node->inputs[1], node->inputs[0], node->inputs[0], node->inputs[1], node->inputs[1], node->inputs[1], node->inputs[1], node->inputs[0], node->inputs[0], node->inputs[1], node->inputs[1], node->inputs[1], node->inputs[1]); break;
                            case 4: sprintf(temp, "vec4(node_%02i.x - dot(node_%02i, node_%02i)/dot(node_%02i, node_%02i)*node_%02i.x, node_%02i.y - dot(node_%02i, node_%02i)/dot(node_%02i, node_%02i)*node_%02i.y, node_%02i.z - dot(node_%02i, node_%02i)/dot(node_%02i, node_%02i)*node_%02i.z, node_%02i.w - dot(node_%02i, node_%02i)/dot(node_%02i, node_%02i)*node_%02i.w);\n", 
                            node->inputs[0], node->inputs[0], node->inputs[1], node->inputs[1], node->inputs[1], node->inputs[1], node->inputs[0], node->inputs[0], node->inputs[1], node->inputs[1], node->inputs[1], node->inputs[1], node->inputs[0], node->inputs[0], node->inputs[1], node->inputs[1], node->inputs[1], node->inputs[1], node->inputs[0], node->inputs[0], node->inputs[1], node->inputs[1], node->inputs[1], node->inputs[1]); break;
                            default: break;
                        }
                    } break;
                    case FNODE_HALFDIRECTION: sprintf(temp, "normalize(node_%02i + node_%02i);\n", node->inputs[0], node->inputs[1]); break;
                    default: break;
                }

                strcat(body, temp);
            }

            // Write current node string to shader file
            fprintf(file, body);
        }
    }
}

// Updates a node shapes due to drag behaviour
void UpdateNodeShapes(FNode node)
{
    if (node != NULL)
    {
        int index = -1;
        int currentLength = 0;
        for (int i = 0; i < node->output.dataCount; i++)
        {
            node->output.data[i].shape.x = node->shape.x + 5;
            node->output.data[i].shape.width = NODE_DATA_WIDTH;
            node->output.data[i].shape.width += MeasureText(node->output.data[i].valueText, 20);

            if (i == 0) node->output.data[i].shape.y = node->shape.y + 5;
            else node->output.data[i].shape.y = node->output.data[i - 1].shape.y + node->output.data[i - 1].shape.height + 5;

            int length = MeasureText(node->output.data[i].valueText, 20);
            if (length > currentLength)
            {
                index = i;
                currentLength = length;
            }
        }

        node->shape.width = 10 + NODE_DATA_WIDTH;
        if (index != -1) node->shape.width += MeasureText(node->output.data[index].valueText, 20);
        else if (node->output.dataCount > 0)
        {
            bool isError = false;

            for (int i = 0; i < node->output.dataCount; i++)
            {
                if (node->output.data[i].valueText[0] != '\0') isError = true;
            }

            if (isError) TraceLogFNode(true, "error trying to calculate node data longest value");
        }

        if (node->type >= FNODE_MATRIX) node->shape.height = ((node->output.dataCount == 0) ? (NODE_DATA_HEIGHT/2 + 10) : ((NODE_DATA_HEIGHT + 5)*node->output.dataCount + 5));
        
        if (node->type >= FNODE_ADD)
        {

            node->inputShape.x = node->shape.x - 20;
            node->inputShape.y = node->shape.y + node->shape.height/2 - 10;
        }

        node->outputShape.x = node->shape.x + node->shape.width;
        node->outputShape.y = node->shape.y + node->shape.height/2 - 10;
    }
    else TraceLogFNode(true, "error trying to calculate values for a null referenced node");
}

// Updates a comment shapes due to drag behaviour
void UpdateCommentShapes(FComment comment)
{
    if (comment != NULL)
    {
        if (commentState == 0)
        {
            if (comment->shape.width < MIN_COMMENT_SIZE) comment->shape.width = MIN_COMMENT_SIZE;
            if (comment->shape.height < MIN_COMMENT_SIZE) comment->shape.height = MIN_COMMENT_SIZE;
        }

        comment->valueShape.x = comment->shape.x + 10;
        comment->valueShape.y = comment->shape.y - UI_COMMENT_HEIGHT - 5;
        comment->sizeTShape.x = comment->shape.x + comment->shape.width/2 - comment->sizeTShape.width/2;
        comment->sizeTShape.y = comment->shape.y - comment->sizeTShape.height/2;
        comment->sizeBShape.x = comment->shape.x + comment->shape.width/2 - comment->sizeBShape.width/2;
        comment->sizeBShape.y = comment->shape.y + comment->shape.height - comment->sizeBShape.height/2;
        comment->sizeLShape.x = comment->shape.x - comment->sizeLShape.width/2;
        comment->sizeLShape.y = comment->shape.y + comment->shape.height/2 - comment->sizeLShape.height/2;
        comment->sizeRShape.x = comment->shape.x + comment->shape.width - comment->sizeRShape.width/2;
        comment->sizeRShape.y = comment->shape.y + comment->shape.height/2 - comment->sizeRShape.height/2;
        comment->sizeTlShape.x = comment->shape.x - comment->sizeTlShape.width/2;
        comment->sizeTlShape.y = comment->shape.y - comment->sizeTlShape.height/2;
        comment->sizeTrShape.x = comment->shape.x + comment->shape.width - comment->sizeTrShape.width/2;
        comment->sizeTrShape.y = comment->shape.y - comment->sizeTrShape.height/2;
        comment->sizeBlShape.x = comment->shape.x - comment->sizeBlShape.width/2;
        comment->sizeBlShape.y = comment->shape.y + comment->shape.height - comment->sizeBlShape.height/2;
        comment->sizeBrShape.x = comment->shape.x + comment->shape.width - comment->sizeBrShape.width/2;
        comment->sizeBrShape.y = comment->shape.y + comment->shape.height - comment->sizeBrShape.height/2;
    }
    else TraceLogFNode(true, "error trying to calculate values for a null referenced comment");
}

// Aligns a node to the nearest grid intersection
void AlignNode(FNode node)
{
    int spacing = 0;
    float currentDistance = 999999;
    int currentSpacing = 0;

    for (int j = 0; j < UI_GRID_COUNT*5; j++)
    {
        float temp = abs(node->shape.x - (-(UI_GRID_COUNT/2*UI_GRID_SPACING*5) + spacing));

        if (currentDistance > temp)
        {
            currentDistance = temp;
            currentSpacing = spacing;
        }

        spacing += UI_GRID_SPACING;
    }

    if (node->shape.x - (-(UI_GRID_COUNT/2*UI_GRID_SPACING*5) + currentSpacing) > 0.0f) node->shape.x -= currentDistance;
    else node->shape.x += currentDistance;

    spacing = 0;
    currentDistance = 999999;
    currentSpacing = 0;
    for (int j = 0; j < UI_GRID_COUNT*5; j++)
    {
        float temp = abs(node->shape.y - (-(UI_GRID_COUNT/2*UI_GRID_SPACING*5) + spacing));

        if (currentDistance > temp)
        {
            currentDistance = temp;
            currentSpacing = spacing;
        }

        spacing += UI_GRID_SPACING;
    }

    if (node->shape.y - (-(UI_GRID_COUNT/2*UI_GRID_SPACING*5) + currentSpacing) > 0.0f) node->shape.y -= currentDistance;
    else node->shape.y += currentDistance;
}

// Aligns all created nodes
void AlignAllNodes()
{
    for (int i = 0; i < nodesCount; i++)
    {
        AlignNode(nodes[i]);
        UpdateNodeShapes(nodes[i]);
    }
}

// Destroys all unused nodes
void ClearUnusedNodes()
{
    for (int i = nodesCount - 1; i >= 0; i--)
    {
        bool used = (nodes[i]->type >= FNODE_VERTEX);

        if (!used)
        {
            for (int k = 0; k < linesCount; k++)
            {
                if ((nodes[i]->id == lines[k]->from) || (nodes[i]->id == lines[k]->to))
                {
                    used = true;
                    break;
                }
            }
        }

        if (!used) DestroyNode(nodes[i]);
    }

    TraceLogFNode(false, "all unused nodes have been deleted [USED RAM: %i bytes]", usedMemory);
}

// Destroys all created nodes and its linked lines
void ClearGraph()
{
    for (int i = nodesCount - 1; i >= 0; i--)
    {
        if (nodes[i]->type < FNODE_VERTEX) DestroyNode(nodes[i]);
    }
    for (int i = commentsCount - 1; i >= 0; i--) DestroyComment(comments[i]);

    TraceLogFNode(false, "all nodes have been deleted [USED RAM: %i bytes]", usedMemory);
}

// Draw canvas space to create nodes
void DrawCanvas()
{    
    // Draw background title and credits
    DrawText("FNODE 1.0", (canvasSize.x - MeasureText("FNODE 1.0", 120))/2, canvasSize.y/2 - 60, 120, Fade(LIGHTGRAY, UI_GRID_ALPHA*2));
    DrawText("VICTOR FISAC", (canvasSize.x - MeasureText("VICTOR FISAC", 40))/2, canvasSize.y*0.65f - 20, 40, Fade(LIGHTGRAY, UI_GRID_ALPHA*2));

    Begin2dMode(camera);

        DrawCanvasGrid(UI_GRID_COUNT);

        // Draw all created comments, lines and nodes
        for (int i = 0; i < commentsCount; i++) DrawComment(comments[i]);
        for (int i = 0; i < nodesCount; i++) DrawNode(nodes[i]);
        for (int i = 0; i < linesCount; i++) DrawNodeLine(lines[i]);

    End2dMode();
}

// Draw canvas grid with a specific number of divisions for horizontal and vertical lines
void DrawCanvasGrid(int divisions)
{
    int spacing = 0;
    for (int i = 0; i < divisions; i++)
    {
        for (int k = 0; k < 5; k++)
        {
            DrawRectangle(-(divisions/2*UI_GRID_SPACING*5) + spacing, -100000, 1, 200000, ((k == 0) ? Fade(BLACK, UI_GRID_ALPHA*2) : Fade(GRAY, UI_GRID_ALPHA)));
            spacing += UI_GRID_SPACING;
        }
    }

    spacing = 0;
    for (int i = 0; i < divisions; i++)
    {
        for (int k = 0; k < 5; k++)
        {
            DrawRectangle(-100000, -(divisions/2*UI_GRID_SPACING*5) + spacing, 200000, 1, ((k == 0) ? Fade(BLACK, UI_GRID_ALPHA*2) : Fade(GRAY, UI_GRID_ALPHA)));
            spacing += UI_GRID_SPACING;
        }
    }
}

// Draws a visor with default model rotating and current shader
void DrawVisor()
{
    BeginTextureMode(visorTarget);
    
        DrawRectangle(0, 0, screenSize.x, screenSize.y, GRAY);

        Begin3dMode(camera3d);

            DrawModelEx(model, (Vector3){ 0.0f, 0.0f, 0.0f }, (Vector3){ 0, 1, 0 }, modelRotation, (Vector3){ 0.13f, 0.13f, 0.13f }, WHITE);

        End3dMode();

    EndTextureMode();

    Rectangle visor = { canvasSize.x - visorTarget.texture.width - UI_PADDING, screenSize.y - visorTarget.texture.height - UI_PADDING, visorTarget.texture.width, visorTarget.texture.height };
    DrawRectangle(visor.x - VISOR_BORDER, visor.y - VISOR_BORDER, visor.width + VISOR_BORDER*2, visor.height + VISOR_BORDER*2, BLACK);

    BeginShaderMode(fxaa);

        DrawTexturePro(visorTarget.texture, (Rectangle){ 0, 0, visorTarget.texture.width, -visorTarget.texture.height }, visor, (Vector2){ 0, 0 }, 0.0f, WHITE);

    EndShaderMode();
}

// Draw interface to create nodes
void DrawInterface()
{
    // Draw interface background
    DrawRectangleRec((Rectangle){ canvasSize.x, 0.0f, screenSize.x - canvasSize.x, screenSize.y }, DARKGRAY);

    // Draw interface main buttons
    if (FButton((Rectangle){ UI_PADDING, screenSize.y - (UI_BUTTON_HEIGHT + UI_PADDING), (screenSize.x - canvasSize.x - UI_PADDING*2)/2, UI_BUTTON_HEIGHT }, "Compile")) CompileShader(); menuOffset = 1;
    if (FButton((Rectangle){ UI_PADDING + ((screenSize.x - canvasSize.x - UI_PADDING*2)/2 + UI_PADDING)*menuOffset, screenSize.y - (UI_BUTTON_HEIGHT + UI_PADDING), (screenSize.x - canvasSize.x - UI_PADDING*2)/2, UI_BUTTON_HEIGHT }, "Clear Graph")) ClearGraph();
    if (FButton((Rectangle){ UI_PADDING + ((screenSize.x - canvasSize.x - UI_PADDING*2)/2 + UI_PADDING)*menuOffset, screenSize.y - (UI_BUTTON_HEIGHT + UI_PADDING), (screenSize.x - canvasSize.x - UI_PADDING*2)/2, UI_BUTTON_HEIGHT }, "Align Nodes")) AlignAllNodes();
    if (FButton((Rectangle){ UI_PADDING + ((screenSize.x - canvasSize.x - UI_PADDING*2)/2 + UI_PADDING)*menuOffset, screenSize.y - (UI_BUTTON_HEIGHT + UI_PADDING), (screenSize.x - canvasSize.x - UI_PADDING*2)/2, UI_BUTTON_HEIGHT }, "Clear Unused Nodes")) ClearUnusedNodes();

    // Draw interface nodes buttons
    DrawText("Constant Vectors", canvasSize.x + ((screenSize.x - canvasSize.x) - MeasureText("Constant Vectors", 10))/2 - UI_PADDING_SCROLL/2, UI_PADDING*4 - menuScroll, 10, WHITE); menuOffset = 1;
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Value")) CreateNodeValue((float)GetRandomValue(-11, 10));
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Vector 2")) CreateNodeVector2((Vector2){ (float)GetRandomValue(0, 10), (float)GetRandomValue(0, 10) });
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Vector 3")) CreateNodeVector3((Vector3){ (float)GetRandomValue(0, 10), (float)GetRandomValue(0, 10), (float)GetRandomValue(0, 10) });
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Vector 4")) CreateNodeVector4((Vector4){ (float)GetRandomValue(0, 10), (float)GetRandomValue(0, 10), (float)GetRandomValue(0, 10), (float)GetRandomValue(0, 10) });
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Matrix 4x4")) CreateNodeMatrix(FMatrixIdentity());

    DrawText("Arithmetic", canvasSize.x + ((screenSize.x - canvasSize.x) - MeasureText("Arithmetic", 10))/2 - UI_PADDING_SCROLL/2, UI_PADDING*4 + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, 10, WHITE); menuOffset++;
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Add")) CreateNodeOperator(FNODE_ADD, "Add", MAX_INPUTS);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Subtract")) CreateNodeOperator(FNODE_SUBTRACT, "Subtract", MAX_INPUTS);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Multiply")) CreateNodeOperator(FNODE_MULTIPLY, "Multiply", MAX_INPUTS);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Multiply Matrix")) CreateNodeOperator(FNODE_MULTIPLYMATRIX, "Multiply Matrix", 2);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Divide")) CreateNodeOperator(FNODE_DIVIDE, "Divide", MAX_INPUTS);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "One Minus")) CreateNodeOperator(FNODE_ONEMINUS, "One Minus", 1);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Abs")) CreateNodeOperator(FNODE_ABS, "Abs", 1);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Clamp 0-1")) CreateNodeOperator(FNODE_CLAMP01, "Clamp 0-1", 1);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Max")) CreateNodeOperator(FNODE_MAX, "Max", 2);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Min")) CreateNodeOperator(FNODE_MIN, "Min", 2);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Negate")) CreateNodeOperator(FNODE_NEGATE, "Negate", 1);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Reciprocal")) CreateNodeOperator(FNODE_RECIPROCAL, "Reciprocal", 1);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Square Root")) CreateNodeOperator(FNODE_SQRT, "Square Root", 1);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Power")) CreateNodeOperator(FNODE_POWER, "Power", 2);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Exp 2")) CreateNodeOperator(FNODE_EXP2, "Exp 2", 1);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Posterize")) CreateNodeOperator(FNODE_POSTERIZE, "Posterize", 2);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Ceil")) CreateNodeOperator(FNODE_CEIL, "Ceil", 1);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Round")) CreateNodeOperator(FNODE_ROUND, "Round", 1);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Trunc")) CreateNodeOperator(FNODE_TRUNC, "Trunc", 1);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Lerp")) CreateNodeOperator(FNODE_LERP, "Lerp", 3);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Step")) CreateNodeOperator(FNODE_STEP, "Step", 2);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "SmoothStep")) CreateNodeOperator(FNODE_SMOOTHSTEP, "SmoothStep", 3);

    DrawText("Vector Operations", canvasSize.x + ((screenSize.x - canvasSize.x) - MeasureText("Vector Operations", 10))/2 - UI_PADDING_SCROLL/2, UI_PADDING*4 + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, 10, WHITE); menuOffset++;
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Append")) CreateNodeOperator(FNODE_APPEND, "Append", 4);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Normalize")) CreateNodeOperator(FNODE_NORMALIZE, "Normalize", 1);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Cross Product")) CreateNodeOperator(FNODE_CROSSPRODUCT, "Cross Product", 2);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Desaturate")) CreateNodeOperator(FNODE_DESATURATE, "Desaturate", 2);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Distance")) CreateNodeOperator(FNODE_DISTANCE, "Distance", 2);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Dot Product")) CreateNodeOperator(FNODE_DOTPRODUCT, "Dot Product", 2);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Length")) CreateNodeOperator(FNODE_LENGTH, "Length", 1);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Transpose")) CreateNodeOperator(FNODE_TRANSPOSE, "Transpose", 1);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Vector Projection")) CreateNodeOperator(FNODE_PROJECTION, "Vector Projection", 2);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Vector Rejection")) CreateNodeOperator(FNODE_REJECTION, "Vector Rejection", 2);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Half Direction")) CreateNodeOperator(FNODE_HALFDIRECTION, "Half Direction", 2);

    DrawText("Geometry Data", canvasSize.x + ((screenSize.x - canvasSize.x) - MeasureText("Geometry Data", 10))/2 - UI_PADDING_SCROLL/2, UI_PADDING*4 + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, 10, WHITE); menuOffset++;
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Vertex Position")) CreateNodeUniform(FNODE_VERTEXPOSITION, "Vertex Position", 3);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Normal Direction")) CreateNodeUniform(FNODE_VERTEXNORMAL, "Normal Direction", 3);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "View Direction")) CreateNodeUniform(FNODE_VIEWDIRECTION, "View Direction", 3);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Fresnel")) CreateNodeUniform(FNODE_FRESNEL, "Fresnel", 1);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "MVP Matrix")) CreateNodeUniform(FNODE_MVP, "MVP Matrix", 16);

    DrawText("Math Constants", canvasSize.x + ((screenSize.x - canvasSize.x) - MeasureText("Math Constants", 10))/2 - UI_PADDING_SCROLL/2, UI_PADDING*4 + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, 10, WHITE); menuOffset++;
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "PI")) CreateNodePI();
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "e")) CreateNodeE();

    DrawText("Trigonometry", canvasSize.x + ((screenSize.x - canvasSize.x) - MeasureText("Trigonometry", 10))/2 - UI_PADDING_SCROLL/2, UI_PADDING*4 + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, 10, WHITE); menuOffset++;
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Cosine")) CreateNodeOperator(FNODE_COS, "Cosine", 1);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Sine")) CreateNodeOperator(FNODE_SIN, "Sine", 1);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Tangent")) CreateNodeOperator(FNODE_TAN, "Tangent", 1);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Deg to Rad")) CreateNodeOperator(FNODE_DEG2RAD, "Deg to Rad", 1);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Rad to Deg")) CreateNodeOperator(FNODE_RAD2DEG, "Rad to Deg", 1);

    DrawRectangle(menuScrollRec.x - 3, 2, menuScrollRec.width + 6, screenSize.y - 4, (Color){ UI_BORDER_DEFAULT_COLOR, UI_BORDER_DEFAULT_COLOR, UI_BORDER_DEFAULT_COLOR, 255 });
    DrawRectangle(menuScrollRec.x - 2, menuScrollRec.y - 2, menuScrollRec.width + 4, menuScrollRec.height + 4, DARKGRAY);
    DrawRectangleRec(menuScrollRec, ((scrollState == 1) ? LIGHTGRAY : RAYWHITE));

    if (debugMode)
    {
        const char *string = 
        "loadedShader: %i\n"
        "selectedNode: %i\n"
        "editNode: %i\n"
        "lineState: %i\n"
        "commentState: %i\n"
        "selectedComment: %i\n"
        "editSize: %i\n"
        "editSizeType: %i\n"
        "editComment: %i\n"
        "editNodeText: %s";

        DrawText(FormatText(string, loadedShader, selectedNode, editNode, lineState, commentState, selectedComment, editSize, editSizeType, editComment, ((editNodeText != NULL) ? editNodeText : "NULL")), 10, 30, 10, BLACK);

        DrawFPS(10, 10);
    }
}

// Converts Vector2 coordinates from world space to Camera2D space based on its offset
Vector2 CameraToViewVector2(Vector2 vector, Camera2D camera)
{
    return (Vector2){ vector.x - camera.offset.x, vector.y - camera.offset.y };
}

// Converts rectangle coordinates from world space to Camera2D space based on its offset
Rectangle CameraToViewRec(Rectangle rec, Camera2D camera)
{
    return (Rectangle){ rec.x + camera.offset.x, rec.y + camera.offset.y, rec.width, rec.height };
}

// Initializes FNode global variables
void InitFNode()
{
    nodesCount = 0;
    linesCount = 0;
    commentsCount = 0;
    selectedCommentNodesCount = 0;
    for (int i = 0; i < MAX_NODES; i++) selectedCommentNodes[i] = -1;

    Shader previousShader = LoadShader(VERTEX_PATH, FRAGMENT_PATH);
    if (previousShader.id != 0)
    {
        shader = previousShader;
        model.material.shader = shader;
        viewUniform = GetShaderLocation(shader, "viewDirection");
        transformUniform = GetShaderLocation(shader, "modelMatrix");
        
        FILE *dataFile = fopen(DATA_PATH, "r");
        if (dataFile != NULL)
        {
            float type;
            float inputs[MAX_INPUTS];
            float inputsCount;
            float inputsLimit;
            float dataCount;
            float data[MAX_VALUES];
            float shapeX;
            float shapeY;
            while (fscanf(dataFile, "%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,\n", &type,
            &inputs[0], &inputs[1], &inputs[2], &inputs[3], &inputsCount, &inputsLimit, &dataCount, &data[0], &data[1], &data[2],
            &data[3], &data[4], &data[5], &data[6], &data[7], &data[8], &data[9], &data[10], &data[11], &data[12], &data[13], &data[14],
            &data[15], &shapeX, &shapeY) > 0)
            {                
                FNode newNode = InitializeNode(true);
                
                newNode->type = type;
                if (type < FNODE_ADD) newNode->inputShape = (Rectangle){ 0, 0, 0, 0 };
                switch ((int)type)
                {
                    case FNODE_PI: newNode->name = "Pi"; break;
                    case FNODE_E: newNode->name = "e"; break;
                    case FNODE_VERTEXPOSITION: newNode->name = "Vertex Position"; break;
                    case FNODE_VERTEXNORMAL: newNode->name = "Normal Direction"; break;
                    case FNODE_FRESNEL: newNode->name = "Fresnel"; break;
                    case FNODE_VIEWDIRECTION: newNode->name = "View Direction"; break;
                    case FNODE_MVP: newNode->name = "MVP Matrix"; break;
                    case FNODE_MATRIX: newNode->name = "Matrix 4x4"; break;
                    case FNODE_VALUE: newNode->name = "Value"; break;
                    case FNODE_VECTOR2: newNode->name = "Vector 2"; break;
                    case FNODE_VECTOR3: newNode->name = "Vector 3"; break;
                    case FNODE_VECTOR4: newNode->name = "Vector 4"; break;
                    case FNODE_ADD: newNode->name = "Add"; break;
                    case FNODE_SUBTRACT: newNode->name = "Subtract"; break;
                    case FNODE_MULTIPLY: newNode->name = "Multiply"; break;
                    case FNODE_DIVIDE: newNode->name = "Divide"; break;
                    case FNODE_APPEND: newNode->name = "Append"; break;
                    case FNODE_ONEMINUS: newNode->name = "One Minus"; break;
                    case FNODE_ABS: newNode->name = "Abs"; break;
                    case FNODE_COS:newNode->name = "Cos"; break;
                    case FNODE_SIN: newNode->name = "Sin"; break;
                    case FNODE_TAN: newNode->name = "Tan"; break;
                    case FNODE_DEG2RAD: newNode->name = "Deg to Rad"; break;
                    case FNODE_RAD2DEG: newNode->name = "Rad to Deg"; break;
                    case FNODE_NORMALIZE: newNode->name = "Normalize"; break;
                    case FNODE_NEGATE: newNode->name = "Negate"; break;
                    case FNODE_RECIPROCAL: newNode->name = "Reciprocal"; break;
                    case FNODE_SQRT: newNode->name = "Square Root"; break;
                    case FNODE_TRUNC: newNode->name = "Trunc"; break;
                    case FNODE_ROUND: newNode->name = "Round"; break;
                    case FNODE_CEIL: newNode->name = "Ceil"; break;
                    case FNODE_CLAMP01: newNode->name = "Clamp 0-1"; break;
                    case FNODE_EXP2: newNode->name = "Exp 2"; break;
                    case FNODE_POWER: newNode->name = "Power"; break;
                    case FNODE_STEP: newNode->name = "Step"; break;
                    case FNODE_POSTERIZE: newNode->name = "Posterize"; break;
                    case FNODE_MAX: newNode->name = "Max"; break;
                    case FNODE_MIN: newNode->name = "Min"; break;
                    case FNODE_LERP: newNode->name = "Lerp"; break;
                    case FNODE_SMOOTHSTEP: newNode->name = "Smooth Step"; break;
                    case FNODE_CROSSPRODUCT: newNode->name = "Cross Product"; break;
                    case FNODE_DESATURATE: newNode->name = "Desaturate"; break;
                    case FNODE_DISTANCE: newNode->name = "Distance"; break;
                    case FNODE_DOTPRODUCT: newNode->name = "Dot Product"; break;
                    case FNODE_LENGTH: newNode->name = "Length"; break;
                    case FNODE_MULTIPLYMATRIX: newNode->name = "Multiply Matrix"; break;
                    case FNODE_TRANSPOSE: newNode->name = "Transpose"; break;
                    case FNODE_PROJECTION: newNode->name = "Projection Vector"; break;
                    case FNODE_REJECTION: newNode->name = "Rejection Vector"; break;
                    case FNODE_HALFDIRECTION: newNode->name = "Half Direction"; break;
                    case FNODE_VERTEX: newNode->name = "Final Vertex Position"; break;
                    case FNODE_FRAGMENT: newNode->name = "Final Fragment Color"; break;
                    default: break;
                }
                
                for (int i = 0; i < MAX_INPUTS; i++) newNode->inputs[i] = inputs[i];
                newNode->inputsCount = inputsCount;
                newNode->inputsLimit = inputsLimit;
                
                for (int i = 0; i < MAX_VALUES; i++)
                {
                    newNode->output.data[i].value = data[i];
                    FFloatToString(newNode->output.data[i].valueText, newNode->output.data[i].value);
                }
                newNode->output.dataCount = dataCount;
                
                newNode->shape.x = shapeX;
                newNode->shape.y = shapeY;
                
                UpdateNodeShapes(newNode);
            }
            
            int from;
            int to;
            int count = 0;
            
            while (fscanf(dataFile, "?%i?%i\n", &from, &to) > 0)
            {
                tempLine = CreateNodeLine(from);
                tempLine->to = to;
            }

            for (int i = 0; i < nodesCount; i++) UpdateNodeShapes(nodes[i]);
            CalculateValues();
            for (int i = 0; i < nodesCount; i++) UpdateNodeShapes(nodes[i]);
            
            loadedShader = true;
            fclose(dataFile);
        }
        else TraceLogFNode(false, "error when trying to open previous shader data file");
    }
    
    if (!loadedShader)
    {
        CreateNodeMaterial(FNODE_VERTEX, "Final Vertex Position", 0);
        CreateNodeMaterial(FNODE_FRAGMENT, "Final Fragment Color", 0);
    }

    TraceLogFNode(false, "initialization complete");
}

// Creates a node which returns PI value
FNode CreateNodePI()
{
    FNode newNode = InitializeNode(false);

    newNode->type = FNODE_PI;
    newNode->name = "PI";
    newNode->output.dataCount = 1;
    newNode->output.data[0].value = PI;
    FFloatToString(newNode->output.data[0].valueText, newNode->output.data[0].value);
    newNode->inputsLimit = 0;

    UpdateNodeShapes(newNode);

    return newNode;
}

// Creates a node which returns e value
FNode CreateNodeE()
{
    FNode newNode = InitializeNode(false);

    newNode->type = FNODE_E;
    newNode->name = "e";
    newNode->output.dataCount = 1;
    newNode->output.data[0].value = 2.71828182845904523536;
    FFloatToString(newNode->output.data[0].valueText, newNode->output.data[0].value);
    newNode->inputsLimit = 0;

    UpdateNodeShapes(newNode);

    return newNode;
}

// Creates a matrix 4x4 node (OpenGL style 4x4 - right handed, column major)
FNode CreateNodeMatrix(Matrix mat)
{
    FNode newNode = InitializeNode(false);

    newNode->type = FNODE_MATRIX;
    newNode->name = "Matrix (4x4)";
    newNode->output.dataCount = 16;
    newNode->output.data[0].value = mat.m0;
    newNode->output.data[1].value = mat.m1;
    newNode->output.data[2].value = mat.m2;
    newNode->output.data[3].value = mat.m3;
    newNode->output.data[4].value = mat.m4;
    newNode->output.data[5].value = mat.m5;
    newNode->output.data[6].value = mat.m6;
    newNode->output.data[7].value = mat.m7;
    newNode->output.data[8].value = mat.m8;
    newNode->output.data[9].value = mat.m9;
    newNode->output.data[10].value = mat.m10;
    newNode->output.data[11].value = mat.m11;
    newNode->output.data[12].value = mat.m12;
    newNode->output.data[13].value = mat.m13;
    newNode->output.data[14].value = mat.m14;
    newNode->output.data[15].value = mat.m15;
    for (int i = 0; i < newNode->output.dataCount; i++) FFloatToString(newNode->output.data[i].valueText, newNode->output.data[i].value);
    newNode->shape.height = (NODE_DATA_HEIGHT + 5)*newNode->output.dataCount + 5;
    newNode->inputsLimit = 0;

    UpdateNodeShapes(newNode);
}

// Creates a value node (1 float)
FNode CreateNodeValue(float value)
{
    FNode newNode = InitializeNode(false);

    newNode->type = FNODE_VALUE;
    newNode->name = "Value";
    newNode->output.dataCount = 1;
    newNode->output.data[0].value = value;
    FFloatToString(newNode->output.data[0].valueText, newNode->output.data[0].value);
    newNode->inputsLimit = 0;

    UpdateNodeShapes(newNode);

    return newNode;
}

// Creates a Vector2 node (2 float)
FNode CreateNodeVector2(Vector2 vector)
{
    FNode newNode = InitializeNode(false);

    newNode->type = FNODE_VECTOR2;
    newNode->name = "Vector 2";
    newNode->output.dataCount = 2;
    newNode->output.data[0].value = vector.x;
    newNode->output.data[1].value = vector.y;
    for (int i = 0; i < newNode->output.dataCount; i++) FFloatToString(newNode->output.data[i].valueText, newNode->output.data[i].value);
    newNode->shape.height = (NODE_DATA_HEIGHT + 5)*newNode->output.dataCount + 5;
    newNode->inputsLimit = 0;

    UpdateNodeShapes(newNode);
}

// Creates a Vector3 node (3 float)
FNode CreateNodeVector3(Vector3 vector)
{
    FNode newNode = InitializeNode(false);

    newNode->type = FNODE_VECTOR3;
    newNode->name = "Vector 3";
    newNode->output.dataCount = 3;
    newNode->output.data[0].value = vector.x;
    newNode->output.data[1].value = vector.y;
    newNode->output.data[2].value = vector.z;
    for (int i = 0; i < newNode->output.dataCount; i++) FFloatToString(newNode->output.data[i].valueText, newNode->output.data[i].value);
    newNode->shape.height = (NODE_DATA_HEIGHT + 5)*newNode->output.dataCount + 5;
    newNode->inputsLimit = 0;

    UpdateNodeShapes(newNode);
}

// Creates a Vector4 node (4 float)
FNode CreateNodeVector4(Vector4 vector)
{
    FNode newNode = InitializeNode(false);

    newNode->type = FNODE_VECTOR4;
    newNode->name = "Vector 4";
    newNode->output.dataCount = 4;
    newNode->output.data[0].value = vector.x;
    newNode->output.data[1].value = vector.y;
    newNode->output.data[2].value = vector.z;
    newNode->output.data[3].value = vector.w;
    for (int i = 0; i < newNode->output.dataCount; i++) FFloatToString(newNode->output.data[i].valueText, newNode->output.data[i].value);
    newNode->shape.height = (NODE_DATA_HEIGHT + 5)*newNode->output.dataCount + 5;
    newNode->inputsLimit = 0;

    UpdateNodeShapes(newNode);
}

// Creates an operator node with type name and inputs limit as parameters
FNode CreateNodeOperator(FNodeType type, const char *name, int inputs)
{
    FNode newNode = InitializeNode(true);

    newNode->type = type;
    newNode->name = name;
    newNode->inputsLimit = inputs;

    UpdateNodeShapes(newNode);

    return newNode;
}

// Creates an uniform node with type name and data count as parameters
FNode CreateNodeUniform(FNodeType type, const char *name, int dataCount)
{
    FNode newNode = InitializeNode(false);

    newNode->type = type;
    newNode->name = name;
    newNode->output.dataCount = dataCount;

    UpdateNodeShapes(newNode);

    return newNode;
}

// Creates the main node that contains final material attributes
FNode CreateNodeMaterial(FNodeType type, const char *name, int dataCount)
{
    FNode newNode = InitializeNode(true);

    newNode->type = type;
    newNode->name = name;
    newNode->output.dataCount = dataCount;
    newNode->outputShape.width = 0;
    newNode->outputShape.height = 0;

    UpdateNodeShapes(newNode);

    return newNode;
}

// Initializes a new node with generic parameters
FNode InitializeNode(bool isOperator)
{
    FNode newNode = (FNode)FNODE_MALLOC(sizeof(FNodeData));
    usedMemory += sizeof(FNodeData);

    int id = -1;
    for (int i = 0; i < MAX_NODES; i++)
    {
        int currentId = i;

        // Check if current id already exist in other node
        for (int k = 0; k < nodesCount; k++)
        {
            if (nodes[k]->id == currentId)
            {
                currentId++;
                break;
            }
        }

        // If not exist, set it as new node id
        if (currentId == i)
        {
            id = i;
            break;
        }
    }

    // Initialize node id
    if (id != -1) newNode->id = id;
    else TraceLogFNode(true, "node creation failed because there is any available id");

    // Initialize node inputs and inputs count
    for (int i = 0; i < MAX_INPUTS; i++) newNode->inputs[i] = -1;
    newNode->inputsCount = 0;
    newNode->inputsLimit = MAX_INPUTS;

    // Initialize shapes
    newNode->shape = (Rectangle){ GetRandomValue(-camera.offset.x + 0, -camera.offset.x + screenSize.x*0.85f - 50*4), GetRandomValue(-camera.offset.y + screenSize.y/2 - 20 - 100, camera.offset.y + screenSize.y/2 - 20 + 100), 10 + NODE_DATA_WIDTH, 40 };
    if (isOperator) newNode->inputShape = (Rectangle){ 0, 0, 20, 20 };
    else newNode->inputShape = (Rectangle){ 0, 0, 0, 0 };
    newNode->outputShape = (Rectangle){ 0, 0, 20, 20 };

    AlignNode(newNode);

    // Initialize node output and output values count
    newNode->output.dataCount = 0;

    for (int i = 0; i < MAX_VALUES; i++)
    {
        newNode->output.data[i].value = 0.0f;
        newNode->output.data[i].shape = (Rectangle){ 0, 0, NODE_DATA_WIDTH, NODE_DATA_HEIGHT };
        newNode->output.data[i].valueText = (char *)FNODE_MALLOC(MAX_NODE_LENGTH);
        usedMemory += MAX_NODE_LENGTH;
        for (int k = 0; k < MAX_NODE_LENGTH; k++) newNode->output.data[i].valueText[k] = '\0';
    }

    nodes[nodesCount] = newNode;
    nodesCount++;

    TraceLogFNode(false, "created new node id %i (index: %i) [USED RAM: %i bytes]", newNode->id, (nodesCount - 1), usedMemory);

    return newNode;
}

// Returns the index of a node searching by its id
int GetNodeIndex(int id)
{
    int output = -1;

    for (int i = 0; i < nodesCount; i++)
    {
        if (nodes[i]->id == id)
        {
            output = i;
            break;
        }
    }
    
    if (output == -1) TraceLogFNode(false, "error when trying to find a node index by its id");

    return output;
}

// Creates a line between two nodes
FLine CreateNodeLine(int from)
{
    FLine newLine = (FLine)FNODE_MALLOC(sizeof(FLineData));
    usedMemory += sizeof(FLineData);

    int id = -1;
    for (int i = 0; i < MAX_LINES; i++)
    {
        int currentId = i;

        // Check if current id already exist in other line
        for (int k = 0; k < linesCount; k++)
        {
            if (lines[k]->id == currentId)
            {
                currentId++;
                break;
            }
        }

        // If not exist, set it as new line id
        if (currentId == i)
        {
            id = i;
            break;
        }
    }

    // Initialize line id
    if (id != -1) newLine->id = id;
    else TraceLogFNode(true, "line creation failed because there is any available id");

    // Initialize line input
    newLine->from = from;
    newLine->to = -1;

    lines[linesCount] = newLine;
    linesCount++;

    TraceLogFNode(false, "created new line id %i (index: %i) [USED RAM: %i bytes]", newLine->id, (linesCount - 1), usedMemory);

    return newLine;
}

// Creates a comment
FComment CreateComment()
{
    FComment newComment = (FComment)FNODE_MALLOC(sizeof(FCommentData));
    usedMemory += sizeof(FCommentData);

    int id = -1;
    for (int i = 0; i < MAX_COMMENTS; i++)
    {
        int currentId = i;

        // Check if current id already exist in other comment
        for (int k = 0; k < commentsCount; k++)
        {
            if (comments[k]->id == currentId)
            {
                currentId++;
                break;
            }
        }

        // If not exist, set it as new comment id
        if (currentId == i)
        {
            id = i;
            break;
        }
    }

    // Initialize comment id
    if (id != -1) newComment->id = id;
    else TraceLogFNode(true, "comment creation failed because there is any available id");

    // Initialize comment input
    newComment->value = (char *)FNODE_MALLOC(MAX_COMMENT_LENGTH);
    usedMemory += MAX_COMMENT_LENGTH;
    for (int i = 0; i < MAX_COMMENT_LENGTH; i++)
    {
        if (i == 0) newComment->value[i] = 'C';
        else if (i == 1) newComment->value[i] = 'h';
        else if (i == 2) newComment->value[i] = 'a';
        else if (i == 3) newComment->value[i] = 'n';
        else if (i == 4) newComment->value[i] = 'g';
        else if (i == 5) newComment->value[i] = 'e';
        else if (i == 6) newComment->value[i] = ' ';
        else if (i == 7) newComment->value[i] = 'm';
        else if (i == 8) newComment->value[i] = 'e';
        else newComment->value[i] = '\0';
    }
    
    newComment->shape = (Rectangle){ 0, 0, 0, 0 };
    newComment->valueShape = (Rectangle){ 0, 0, UI_COMMENT_WIDTH, UI_COMMENT_HEIGHT };
    newComment->sizeTShape = (Rectangle){ 0, 0, 40, 10 };
    newComment->sizeBShape = (Rectangle){ 0, 0, 40, 10 };
    newComment->sizeLShape = (Rectangle){ 0, 0, 10, 40 };
    newComment->sizeRShape = (Rectangle){ 0, 0, 10, 40 };
    newComment->sizeTlShape = (Rectangle){ 0, 0, 10, 10 };
    newComment->sizeTrShape = (Rectangle){ 0, 0, 10, 10 };
    newComment->sizeBlShape = (Rectangle){ 0, 0, 10, 10 };
    newComment->sizeBrShape = (Rectangle){ 0, 0, 10, 10 };

    comments[commentsCount] = newComment;
    commentsCount++;

    TraceLogFNode(false, "created new comment id %i (index: %i) [USED RAM: %i bytes]", newComment->id, (commentsCount - 1), usedMemory);

    return newComment;
}

// Draws a previously created node
void DrawNode(FNode node)
{
    if (node != NULL)
    {
        DrawRectangleRec(node->shape, ((node->id == selectedNode) ? GRAY : LIGHTGRAY));
        DrawRectangleLines(node->shape.x, node->shape.y, node->shape.width, node->shape.height, BLACK);
        DrawText(FormatText("%s [ID: %i]", node->name, node->id), node->shape.x + node->shape.width/2 - MeasureText(FormatText("%s [ID: %i]", node->name, node->id), 10)/2, node->shape.y - 15, 10, BLACK);

        if ((node->type >= FNODE_MATRIX) && (node->type <= FNODE_VECTOR4))
        {
            if (node->id == editNode)
            {
                int charac = -1;
                charac = GetKeyPressed();

                if (charac != -1)
                {
                    if (charac == KEY_BACKSPACE)
                    {
                        for (int i = 0; i < MAX_NODE_LENGTH; i++)
                        {
                            if ((node->output.data[editNodeType].valueText[i] == '\0'))
                            {
                                node->output.data[editNodeType].valueText[i - 1] = '\0';
                                break;
                            }
                        }

                        node->output.data[editNodeType].valueText[MAX_NODE_LENGTH - 1] = '\0';

                        UpdateNodeShapes(node);
                    }
                    else if (charac == KEY_ENTER)
                    {
                        // Check new node value requeriments (cannot be empty, ...)
                        bool valuesCheck = (node->output.data[editNodeType].valueText[0] != '\0');
                        if (valuesCheck)
                        {
                            FStringToFloat(&node->output.data[editNodeType].value, (const char*)node->output.data[editNodeType].valueText);
                            FFloatToString(node->output.data[editNodeType].valueText, node->output.data[editNodeType].value);

                            CalculateValues();
                        }
                        else
                        {
                            TraceLogFNode(false, "error when trying to change node id %i value due to invalid characters (%s)", node->id, node->output.data[editNodeType].valueText);
                            for (int i = 0; i < MAX_NODE_LENGTH; i++) node->output.data[editNodeType].valueText[i] = editNodeText[i];
                        }

                        UpdateNodeShapes(node);
                        editNode = -1;
                        editNodeType = -1;
                        FNODE_FREE(editNodeText);
                        usedMemory -= MAX_NODE_LENGTH;
                        editNodeText = NULL;
                    }
                    else
                    {
                        // Check for numbers, dot and dash values
                        if (((charac > 47) && (charac < 58)) || (charac == 45) || (charac == 46))
                        {
                            for (int i = 0; i < MAX_NODE_LENGTH; i++)
                            {
                                if (node->output.data[editNodeType].valueText[i] == '\0')
                                {
                                    node->output.data[editNodeType].valueText[i] = (char)charac;
                                    UpdateNodeShapes(node);
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }

        if ((node->type <= FNODE_E) || (node->type >= FNODE_MATRIX))
        {
            for (int i = 0; i < node->output.dataCount; i++)
            {
                if ((node->type >= FNODE_MATRIX) && (node->type <= FNODE_VECTOR4)) DrawRectangleLines(node->output.data[i].shape.x,node->output.data[i].shape.y, node->output.data[i].shape.width, node->output.data[i].shape.height, (((editNode == node->id) && (editNodeType == i)) ? BLACK : GRAY));
                DrawText(node->output.data[i].valueText, node->output.data[i].shape.x + (node->output.data[i].shape.width - 
                         MeasureText(node->output.data[i].valueText, 20))/2, node->output.data[i].shape.y + 
                         node->output.data[i].shape.height/2 - 9, 20, DARKGRAY);
            }
        }

        if (node->inputsCount > 0) DrawRectangleRec(node->inputShape, ((CheckCollisionPointRec(mousePosition, CameraToViewRec(node->inputShape, camera)) ? LIGHTGRAY : GRAY)));
        else DrawRectangleRec(node->inputShape, ((CheckCollisionPointRec(mousePosition, CameraToViewRec(node->inputShape, camera)) ? LIGHTGRAY : RED)));
        DrawRectangleLines(node->inputShape.x, node->inputShape.y, node->inputShape.width, node->inputShape.height, BLACK);

        DrawRectangleRec(node->outputShape, ((CheckCollisionPointRec(mousePosition, CameraToViewRec(node->outputShape, camera)) ? LIGHTGRAY : GRAY)));
        DrawRectangleLines(node->outputShape.x, node->outputShape.y, node->outputShape.width, node->outputShape.height, BLACK);

        if (debugMode)
        {
            const char *string =
            "id: %i\n"
            "type: %i\n"
            "name: %s\n"
            "inputs(%i): %i, %i, %i, %i\n"
            "output(%i): %.02f, %.02f, %.02f, %.02f\n"
            "outputText: %s, %s, %s, %s\n"
            "shape: %i %i, %i, %i";
            
            DrawText(FormatText(string, node->id, node->type, node->name, node->inputsCount, node->inputs[0], node->inputs[1], node->inputs[2], 
            node->inputs[3], node->output.dataCount, node->output.data[0].value, node->output.data[1].value, node->output.data[2].value, node->output.data[3].value, 
            node->output.data[0].valueText, node->output.data[1].valueText, node->output.data[2].valueText, node->output.data[3].valueText, node->shape.x, node->shape.y, 
            node->shape.width, node->shape.height), node->shape.x, node->shape.y + node->shape.height + 5, 10, BLACK);
        }
    }
    else TraceLogFNode(true, "error trying to draw a null referenced node");
}

// Draws a previously created node line
void DrawNodeLine(FLine line)
{
    if (line != NULL)
    {
        Vector2 from = { 0, 0 };
        Vector2 to = { 0, 0 };

        int indexTo = -1;        
        if (line->to != -1)
        {
            indexTo = GetNodeIndex(line->to);
            
            to.x = nodes[indexTo]->inputShape.x + nodes[indexTo]->inputShape.width/2;
            to.y = nodes[indexTo]->inputShape.y + nodes[indexTo]->inputShape.height/2;
        }
        else to = CameraToViewVector2(mousePosition, camera);

        int indexFrom = GetNodeIndex(line->from);

        from.x = nodes[indexFrom]->outputShape.x + nodes[indexFrom]->outputShape.width/2;
        from.y = nodes[indexFrom]->outputShape.y + nodes[indexFrom]->outputShape.height/2;

        DrawCircle(from.x, from.y, 5, ((tempLine->id == line->id && tempLine->to == -1) ? DARKGRAY : BLACK));
        DrawCircle(to.x, to.y, 5, ((tempLine->id == line->id && tempLine->to == -1) ? DARKGRAY : BLACK));

        if (from.x <= to.x)
        {
            int current = 0;
            while (current < NODE_LINE_DIVISIONS) 
            {
                Vector2 fromCurve = { 0, 0 };
                fromCurve.x = FEaseLinear(current, from.x, to.x - from.x, NODE_LINE_DIVISIONS);
                fromCurve.y = FEaseInOutQuad(current, from.y, to.y - from.y, NODE_LINE_DIVISIONS);
                current++;

                Vector2 toCurve = { 0, 0 };
                toCurve.x = FEaseLinear(current, from.x, to.x - from.x, NODE_LINE_DIVISIONS);
                toCurve.y = FEaseInOutQuad(current, from.y, to.y - from.y, NODE_LINE_DIVISIONS);

                DrawLine(fromCurve.x, fromCurve.y, toCurve.x, toCurve.y, ((tempLine->id == line->id && tempLine->to == -1) ? DARKGRAY : BLACK));
            }
        }
        else
        {
            int x = 0.0f;
            int y = 0.0f;
            float angle = -90.0f;
            float multiplier = (((to.y - from.y) > 0) ? 1 : -1);
            float radius = (fabs(to.y - from.y)/4 + 0.02f)*multiplier;
            float distance = FClamp(fabs(to.x - from.x)/100, 0.0f, 1.0f);

            DrawLine(from.x, from.y, from.x, from.y, BLACK);
            while (angle < 90)
            {
                DrawLine(from.x + FCos(angle*DEG2RAD)*radius*multiplier*distance, from.y + radius + FSin(angle*DEG2RAD)*radius, from.x + FCos((angle + 10)*DEG2RAD)*radius*multiplier*distance, from.y + radius + FSin((angle + 10)*DEG2RAD)*radius, BLACK);
                angle += 10;
            }
            Vector2 lastPosition = { from.x, from.y + radius*2 };
            DrawLine(lastPosition.x, lastPosition.y, to.x + FCos(270*DEG2RAD)*radius*multiplier, to.y - radius + FSin(270*DEG2RAD)*radius, BLACK);
            lastPosition.x = to.x;
            while (angle < 270)
            {
                DrawLine(to.x + FCos(angle*DEG2RAD)*radius*multiplier*distance, to.y - radius + FSin(angle*DEG2RAD)*radius, to.x + FCos((angle + 10)*DEG2RAD)*radius*multiplier*distance, to.y - radius + FSin((angle + 10)*DEG2RAD)*radius, BLACK);
                angle += 10;
            }
            lastPosition.y = lastPosition.y + radius*2;
            DrawLine(to.x, to.y, to.x, to.y, BLACK);
        }
        
        if (indexFrom != -1 && indexTo != -1)
        {
            switch (nodes[indexTo]->type)
            {
                case FNODE_POWER:
                {
                    if ((nodes[indexTo]->inputsCount > 0) && (nodes[indexTo]->inputs[0] == nodes[indexFrom]->id)) DrawText("Input", from.x + 5, from.y - 30, 10, BLACK);
                    else if ((nodes[indexTo]->inputsCount > 1) && (nodes[indexTo]->inputs[1] == nodes[indexFrom]->id)) DrawText("Exponent", from.x + 5, from.y - 30, 10, BLACK);
                } break;
                case FNODE_STEP:
                {
                    if ((nodes[indexTo]->inputsCount > 0) && (nodes[indexTo]->inputs[0] == nodes[indexFrom]->id)) DrawText("Value A", from.x + 5, from.y - 30, 10, BLACK);
                    else if ((nodes[indexTo]->inputsCount > 1) && (nodes[indexTo]->inputs[1] == nodes[indexFrom]->id)) DrawText("Value B", from.x + 5, from.y - 30, 10, BLACK);
                } break;
                case FNODE_POSTERIZE:
                {
                    if ((nodes[indexTo]->inputsCount > 0) && (nodes[indexTo]->inputs[0] == nodes[indexFrom]->id)) DrawText("Input", from.x + 5, from.y - 30, 10, BLACK);
                    else if ((nodes[indexTo]->inputsCount > 1) && (nodes[indexTo]->inputs[1] == nodes[indexFrom]->id)) DrawText("Samples", from.x + 5, from.y - 30, 10, BLACK);
                } break;
                case FNODE_LERP:
                {
                    if ((nodes[indexTo]->inputsCount > 0) && (nodes[indexTo]->inputs[0] == nodes[indexFrom]->id)) DrawText("Value A", from.x + 5, from.y - 30, 10, BLACK);
                    else if ((nodes[indexTo]->inputsCount > 1) && (nodes[indexTo]->inputs[1] == nodes[indexFrom]->id)) DrawText("Value B", from.x + 5, from.y - 30, 10, BLACK);
                    else if ((nodes[indexTo]->inputsCount > 2) && (nodes[indexTo]->inputs[2] == nodes[indexFrom]->id)) DrawText("Time", from.x + 5, from.y - 30, 10, BLACK);
                } break;
                case FNODE_SMOOTHSTEP:
                {
                    if ((nodes[indexTo]->inputsCount > 0) && (nodes[indexTo]->inputs[0] == nodes[indexFrom]->id)) DrawText("Min", from.x + 5, from.y - 30, 10, BLACK);
                    else if ((nodes[indexTo]->inputsCount > 1) && (nodes[indexTo]->inputs[1] == nodes[indexFrom]->id)) DrawText("Max", from.x + 5, from.y - 30, 10, BLACK);
                    else if ((nodes[indexTo]->inputsCount > 2) && (nodes[indexTo]->inputs[2] == nodes[indexFrom]->id)) DrawText("Value", from.x + 5, from.y - 30, 10, BLACK);
                } break;
                case FNODE_DESATURATE:
                {
                    if ((nodes[indexTo]->inputsCount > 0) && (nodes[indexTo]->inputs[0] == nodes[indexFrom]->id)) DrawText("Value", from.x + 5, from.y - 30, 10, BLACK);
                    else if ((nodes[indexTo]->inputsCount > 1) && (nodes[indexTo]->inputs[1] == nodes[indexFrom]->id)) DrawText("Amount (0-1)", from.x + 5, from.y - 30, 10, BLACK);
                } break;
                case FNODE_MULTIPLYMATRIX:
                {
                    if ((nodes[indexTo]->inputsCount > 0) && (nodes[indexTo]->inputs[0] == nodes[indexFrom]->id)) DrawText("Left", from.x + 5, from.y - 30, 10, BLACK);
                    else if ((nodes[indexTo]->inputsCount > 1) && (nodes[indexTo]->inputs[1] == nodes[indexFrom]->id)) DrawText("Right", from.x + 5, from.y - 30, 10, BLACK);
                } break;
                default: break;
            }
        }

        if (debugMode)
        {
            const char *string =
            "id: %i\n"
            "from: %i\n"
            "to: %i\n";
            DrawText(FormatText(string, line->id, line->from, line->to), screenSize.x*0.85f - 10 - 50, 10 + 75*line->id, 10, BLACK);
        }
    }
    else TraceLogFNode(true, "error trying to draw a null referenced line");
}

// Draws a previously created comment
void DrawComment(FComment comment)
{
    if (comment != NULL)
    {
        if ((commentState == 0) || ((commentState == 1) && (tempComment->id != comment->id)) || ((commentState == 1) && editSize != -1)) DrawRectangleRec(comment->shape, Fade(YELLOW, 0.2f));
        DrawRectangleLines(comment->shape.x, comment->shape.y, comment->shape.width, comment->shape.height, BLACK);

        if ((commentState == 0) || ((commentState == 1) && (tempComment->id != comment->id)) || ((commentState == 1) && editSize != -1))
        {
            DrawRectangleRec(comment->sizeTShape, ((CheckCollisionPointRec(mousePosition, CameraToViewRec(comment->sizeTShape, camera))) ? LIGHTGRAY : GRAY));
            DrawRectangleLines(comment->sizeTShape.x, comment->sizeTShape.y, comment->sizeTShape.width, comment->sizeTShape.height, BLACK);
            DrawRectangleRec(comment->sizeBShape, ((CheckCollisionPointRec(mousePosition, CameraToViewRec(comment->sizeBShape, camera))) ? LIGHTGRAY : GRAY));
            DrawRectangleLines(comment->sizeBShape.x, comment->sizeBShape.y, comment->sizeBShape.width, comment->sizeBShape.height, BLACK);
            DrawRectangleRec(comment->sizeLShape, ((CheckCollisionPointRec(mousePosition, CameraToViewRec(comment->sizeLShape, camera))) ? LIGHTGRAY : GRAY));
            DrawRectangleLines(comment->sizeLShape.x, comment->sizeLShape.y, comment->sizeLShape.width, comment->sizeLShape.height, BLACK);
            DrawRectangleRec(comment->sizeRShape, ((CheckCollisionPointRec(mousePosition, CameraToViewRec(comment->sizeRShape, camera))) ? LIGHTGRAY : GRAY));
            DrawRectangleLines(comment->sizeRShape.x, comment->sizeRShape.y, comment->sizeRShape.width, comment->sizeRShape.height, BLACK);
            DrawRectangleRec(comment->sizeTlShape, ((CheckCollisionPointRec(mousePosition, CameraToViewRec(comment->sizeTlShape, camera))) ? LIGHTGRAY : GRAY));
            DrawRectangleLines(comment->sizeTlShape.x, comment->sizeTlShape.y, comment->sizeTlShape.width, comment->sizeTlShape.height, BLACK);
            DrawRectangleRec(comment->sizeTrShape, ((CheckCollisionPointRec(mousePosition, CameraToViewRec(comment->sizeTrShape, camera))) ? LIGHTGRAY : GRAY));
            DrawRectangleLines(comment->sizeTrShape.x, comment->sizeTrShape.y, comment->sizeTrShape.width, comment->sizeTrShape.height, BLACK);
            DrawRectangleRec(comment->sizeBlShape, ((CheckCollisionPointRec(mousePosition, CameraToViewRec(comment->sizeBlShape, camera))) ? LIGHTGRAY : GRAY));
            DrawRectangleLines(comment->sizeBlShape.x, comment->sizeBlShape.y, comment->sizeBlShape.width, comment->sizeBlShape.height, BLACK);
            DrawRectangleRec(comment->sizeBrShape, ((CheckCollisionPointRec(mousePosition, CameraToViewRec(comment->sizeBrShape, camera))) ? LIGHTGRAY : GRAY));
            DrawRectangleLines(comment->sizeBrShape.x, comment->sizeBrShape.y, comment->sizeBrShape.width, comment->sizeBrShape.height, BLACK);
        }

        if (comment->id == editComment)
        {
            int letter = -1;
            letter = GetKeyPressed();

            if (letter != -1)
            {
                if (letter == KEY_BACKSPACE)
                {
                    for (int i = 0; i < MAX_COMMENT_LENGTH; i++)
                    {
                        if ((comment->value[i] == '\0'))
                        {
                            comment->value[i - 1] = '\0';
                            break;
                        }
                    }

                    comment->value[MAX_COMMENT_LENGTH - 1] = '\0';
                }
                else if (letter == KEY_ENTER) editComment = -1;
                else
                {
                    if ((letter >= 32) && (letter < 127))
                    {
                        for (int i = 0; i < MAX_COMMENT_LENGTH; i++)
                        {
                            if (comment->value[i] == '\0')
                            {
                                comment->value[i] = (char)letter;
                                break;
                            }
                        }
                    }
                }
            }
        }

        DrawRectangleLines(comment->valueShape.x, comment->valueShape.y, comment->valueShape.width, comment->valueShape.height, ((editComment == comment->id) ? BLACK : LIGHTGRAY));

        int initPos = comment->shape.x + 14;
        for (int i = 0; i < MAX_COMMENT_LENGTH; i++)
        {
            if (comment->value[i] == '\0') break;
            
            DrawText(FormatText("%c", comment->value[i]), initPos, comment->valueShape.y + 2, 20, DARKGRAY);
            initPos += MeasureText(FormatText("%c", comment->value[i]), 20) + 1;
        }

        if (debugMode)
        {
            const char *string =
            "id: %i\n"
            "value: %s\n"
            "shape: %i, %i, %i, %i\n";
            
            DrawText(FormatText(string, comment->id, comment->value, comment->shape.x, comment->shape.y, comment->shape.width, comment->shape.height), comment->shape.x, comment->shape.y + comment->shape.height + 5, 10, BLACK);
        }
    }
    else TraceLogFNode(true, "error trying to draw a null referenced line");
}

// Button element, returns true when pressed
bool FButton(Rectangle bounds, const char *text)
{
    ButtonState buttonState = BUTTON_DEFAULT;

    if (bounds.width < (MeasureText(text, 10) + 20)) bounds.width = MeasureText(text, 10) + 20;
    if (bounds.height < 10) bounds.height = 10 + 40;

    if (CheckCollisionPointRec(mousePosition, bounds) && (scrollState == 0))
    {
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) buttonState = BUTTON_PRESSED;
        else if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) buttonState = BUTTON_CLICKED;
        else buttonState = BUTTON_HOVER;
    }

    switch (buttonState)
    {
        case BUTTON_DEFAULT:
        {
            DrawRectangleRec(bounds, (Color){ UI_BORDER_DEFAULT_COLOR, UI_BORDER_DEFAULT_COLOR, UI_BORDER_DEFAULT_COLOR, 255 });
            DrawRectangle(bounds.x + 2, bounds.y + 2, bounds.width - 4,bounds.height - 4, UI_BUTTON_DEFAULT_COLOR);
            DrawText(text, bounds.x + ((bounds.width/2) - (MeasureText(text, 10)/2)), bounds.y + (bounds.height - 10)/2, 10, DARKGRAY);
        } break;
        case BUTTON_HOVER:
        {
            DrawRectangleRec(bounds, (Color){ UI_BORDER_DEFAULT_COLOR, UI_BORDER_DEFAULT_COLOR, UI_BORDER_DEFAULT_COLOR, 255 });
            DrawRectangle(bounds.x + 2, bounds.y + 2, bounds.width - 4,bounds.height - 4, UI_BUTTON_DEFAULT_COLOR);
            DrawRectangleRec(bounds, Fade(WHITE, 0.4f));
            DrawText(text, bounds.x + ((bounds.width/2) - (MeasureText(text, 10)/2)), bounds.y + (bounds.height - 10)/2, 10, BLACK);
        } break;
        case BUTTON_PRESSED:
        {
            DrawRectangleRec(bounds, UI_BUTTON_DEFAULT_COLOR);
            DrawRectangle(bounds.x + 2, bounds.y + 2, bounds.width - 4,bounds.height - 4, GRAY);
            DrawText(text, bounds.x + ((bounds.width/2) - (MeasureText(text, 10)/2)), bounds.y + (bounds.height - 10)/2, 10, LIGHTGRAY);
        } break;
        default: break;
    }

    menuOffset++;

    return (buttonState == BUTTON_CLICKED);
}

// Destroys a node and its linked lines
void DestroyNode(FNode node)
{
    if (node != NULL)
    {
        int id = node->id;
        int index = GetNodeIndex(id);

        for (int i = linesCount - 1; i >= 0; i--)
        {
            if (lines[i]->from == node->id || lines[i]->to == node->id) DestroyNodeLine(lines[i]);
        }

        for (int i = 0; i < MAX_VALUES; i++)
        {
           FNODE_FREE(nodes[index]->output.data[i].valueText);
           usedMemory -= MAX_NODE_LENGTH;
           nodes[index]->output.data[i].valueText = NULL;
        }

        FNODE_FREE(nodes[index]);
        usedMemory -= sizeof(FNodeData);
        nodes[index] = NULL;

        for (int i = index; i < nodesCount; i++)
        {
            if ((i + 1) < nodesCount) nodes[i] = nodes[i + 1];
        }

        nodesCount--;

        TraceLogFNode(false, "destroyed node id %i (index: %i) [USED RAM: %i bytes]", id, index, usedMemory);
        
        CalculateValues();
    }
    else TraceLogFNode(true, "error trying to destroy a null referenced node");
}

// Destroys a node line
void DestroyNodeLine(FLine line)
{
    if (line != NULL)
    {
        int id = line->id;
        int index = -1;

        for (int i = 0; i < linesCount; i++)
        {
            if (lines[i]->id == id)
            {
                index = i;
                break;
            }
        }

        if (index != -1)
        {        
            FNODE_FREE(lines[index]);
            usedMemory -= sizeof(FLineData);
            lines[index] = NULL;
            
            for (int i = index; i < linesCount; i++)
            {
                if ((i + 1) < linesCount) lines[i] = lines[i + 1];
            }

            linesCount--;

            TraceLogFNode(false, "destroyed line id %i (index: %i) [USED RAM: %i bytes]", id, index, usedMemory);

            CalculateValues();
        }
        else TraceLogFNode(true, "error when trying to destroy line id %i due to index is out of bounds %i", id, index);
    }
    else TraceLogFNode(true, "error trying to destroy a null referenced line");
}

// Destroys a comment
void DestroyComment(FComment comment)
{
    if (comment != NULL)
    {
        int id = comment->id;
        int index = -1;

        for (int i = 0; i < commentsCount; i++)
        {
            if (comments[i]->id == id)
            {
                index = i;
                break;
            }
        }

        if (index != -1)
        {
            FNODE_FREE(comments[index]->value);
            usedMemory -= MAX_COMMENT_LENGTH;
            comments[index]->value = NULL;
            
            FNODE_FREE(comments[index]);
            usedMemory -= sizeof(FCommentData);
            comments[index] = NULL;
            
            for (int i = index; i < commentsCount; i++)
            {
                if ((i + 1) < commentsCount) comments[i] = comments[i + 1];
            }

            commentsCount--;
            
            TraceLogFNode(false, "destroyed comment id %i (index: %i) [USED RAM: %i bytes]", id, index, usedMemory);
        }
        else TraceLogFNode(true, "error when trying to destroy comment id %i due to index is out of bounds %i", id, index);
    }
    else TraceLogFNode(true, "error trying to destroy a null referenced comment");
}

// Unitializes FNode global variables
void CloseFNode()
{
    for (int i = 0; i < nodesCount; i++)
    {
        if (nodes[i] != NULL)
        {
            for (int k = 0; k < MAX_VALUES; k++)
            {
                if (nodes[i]->output.data[k].valueText != NULL)
                {
                    FNODE_FREE(nodes[i]->output.data[k].valueText);
                    usedMemory -= MAX_NODE_LENGTH;
                    nodes[i]->output.data[k].valueText = NULL;
                }
            }

            FNODE_FREE(nodes[i]);
            usedMemory -= sizeof(FNodeData);
            nodes[i] = NULL;
        }
    }
    
    for (int i = 0; i < linesCount; i++)
    {
        if (lines[i] != NULL)
        {
            FNODE_FREE(lines[i]);
            usedMemory -= sizeof(FLineData);
            lines[i] = NULL;
        }
    }

    for (int i = 0; i < commentsCount; i++)
    {
        if ((comments[i] != NULL) && (comments[i]->value != NULL))
        {
            FNODE_FREE(comments[i]->value);
            usedMemory -= MAX_COMMENT_LENGTH;
            comments[i]->value = NULL;
            
            FNODE_FREE(comments[i]);
            usedMemory -= sizeof(FCommentData);
            comments[i] = NULL;
        }
    }

    if (editNodeText != NULL)
    {
        FNODE_FREE(editNodeText);
        usedMemory -= MAX_NODE_LENGTH;
        editNodeText = NULL;
    }

    nodesCount = 0;
    linesCount = 0;
    commentsCount = 0;
    selectedCommentNodesCount = 0;
    for (int i = 0; i < MAX_NODES; i++) selectedCommentNodes[i] = -1;

    TraceLogFNode(false, "unitialization complete [USED RAM: %i bytes]", usedMemory);
}

// Outputs a trace log message
void TraceLogFNode(bool error, const char *text, ...)
{
    va_list args;

    fprintf(stdout, "FNode: ");
    va_start(args, text);
    vfprintf(stdout, text, args);
    va_end(args);

    fprintf(stdout, "\n");

    if (error) exit(1);
}

// Returns length of a Vector2
float FVector2Length(Vector2 v)
{
    return (float)sqrt(v.x*v.x + v.y*v.y);
}

// Returns length of a Vector3
float FVector3Length(Vector3 v)
{
    return (float)sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
}

// Returns length of a Vector4
float FVector4Length(Vector4 v)
{
    return (float)sqrt(v.x*v.x + v.y*v.y + v.z*v.z + v.w*v.w);
}

// Returns a normalized Vector2
Vector2 FVector2Normalize(Vector2 v)
{
    float length = FVector2Length(v);
    return (Vector2){ v.x/length, v.y/length };
}

// Returns a normalized Vector3
Vector3 FVector3Normalize(Vector3 v)
{
    float length = FVector3Length(v);
    return (Vector3){ v.x/length, v.y/length, v.z/length };
}

// Returns a normalized Vector4
Vector4 FVector4Normalize(Vector4 v)
{
    float length = FVector4Length(v);
    return (Vector4){ v.x/length, v.y/length, v.z/length, v.w/length };
}

// Returns the projection vector of two Vector2
Vector2 FVector2Projection(Vector2 a, Vector2 b)
{
    Vector2 output = { 0, 0 };

    float dotAB = FVector2Dot(a, b);
    float dotBB = FVector2Dot(b, b);
    output.x = dotAB/dotBB*b.x;
    output.y = dotAB/dotBB*b.y;

    return output;
}

// Returns the projection vector of two Vector3
Vector3 FVector3Projection(Vector3 a, Vector3 b)
{
    Vector3 output = { 0, 0, 0 };

    float dotAB = FVector3Dot(a, b);
    float dotBB = FVector3Dot(b, b);
    output.x = dotAB/dotBB*b.x;
    output.y = dotAB/dotBB*b.y;
    output.z = dotAB/dotBB*b.z;

    return output;
}

// Returns the projection vector of two Vector4
Vector4 FVector4Projection(Vector4 a, Vector4 b)
{
    Vector4 output = { 0, 0, 0, 0 };

    float dotAB = FVector4Dot(a, b);
    float dotBB = FVector4Dot(b, b);
    output.x = dotAB/dotBB*b.x;
    output.y = dotAB/dotBB*b.y;
    output.z = dotAB/dotBB*b.z;
    output.w = dotAB/dotBB*b.w;

    return output;
}

// Returns the rejection vector of two Vector2
Vector2 FVector2Rejection(Vector2 a, Vector2 b)
{
    Vector2 output = { 0, 0 };

    Vector2 projection = FVector2Projection(a, b);
    output.x = a.x - projection.x;
    output.y = a.y - projection.y;

    return output;
}

// Returns the rejection vector of two Vector3
Vector3 FVector3Rejection(Vector3 a, Vector3 b)
{
    Vector3 output = { 0, 0, 0 };

    Vector3 projection = FVector3Projection(a, b);
    output.x = a.x - projection.x;
    output.y = a.y - projection.y;
    output.z = a.z - projection.z;

    return output;
}

// Returns the rejection vector of two Vector4
Vector4 FVector4Rejection(Vector4 a, Vector4 b)
{
    Vector4 output = { 0, 0, 0, 0 };

    Vector4 projection = FVector4Projection(a, b);
    output.x = a.x - projection.x;
    output.y = a.y - projection.y;
    output.z = a.z - projection.z;
    output.w = a.w - projection.w;

    return output;
}

// Returns the dot product of two Vector2
float FVector2Dot(Vector2 a, Vector2 b)
{
    return (float)((a.x*b.x) + (a.y*b.y));
}

// Returns the dot product of two Vector3
float FVector3Dot(Vector3 a, Vector3 b)
{
    return (float)((a.x*b.x) + (a.y*b.y) + (a.z*b.z));
}

// Returns the dot product of two Vector4
float FVector4Dot(Vector4 a, Vector4 b)
{
    return (float)((a.x*b.x) + (a.y*b.y) + (a.z*b.z) + (a.w*b.w));
}

// Returns the cross product of two vectors
Vector3 FCrossProduct(Vector3 a, Vector3 b)
{  
    return (Vector3){ a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x };
}

// Returns identity matrix
Matrix FMatrixIdentity()
{
    Matrix result = { 1.0f, 0.0f, 0.0f, 0.0f, 
                      0.0f, 1.0f, 0.0f, 0.0f, 
                      0.0f, 0.0f, 1.0f, 0.0f,
                      0.0f, 0.0f, 0.0f, 1.0f };

    return result;
}

// Returns the result of multiply two matrices
Matrix FMatrixMultiply(Matrix left, Matrix right)
{
    Matrix result;

    result.m0 = right.m0*left.m0 + right.m1*left.m4 + right.m2*left.m8 + right.m3*left.m12;
    result.m1 = right.m0*left.m1 + right.m1*left.m5 + right.m2*left.m9 + right.m3*left.m13;
    result.m2 = right.m0*left.m2 + right.m1*left.m6 + right.m2*left.m10 + right.m3*left.m14;
    result.m3 = right.m0*left.m3 + right.m1*left.m7 + right.m2*left.m11 + right.m3*left.m15;
    result.m4 = right.m4*left.m0 + right.m5*left.m4 + right.m6*left.m8 + right.m7*left.m12;
    result.m5 = right.m4*left.m1 + right.m5*left.m5 + right.m6*left.m9 + right.m7*left.m13;
    result.m6 = right.m4*left.m2 + right.m5*left.m6 + right.m6*left.m10 + right.m7*left.m14;
    result.m7 = right.m4*left.m3 + right.m5*left.m7 + right.m6*left.m11 + right.m7*left.m15;
    result.m8 = right.m8*left.m0 + right.m9*left.m4 + right.m10*left.m8 + right.m11*left.m12;
    result.m9 = right.m8*left.m1 + right.m9*left.m5 + right.m10*left.m9 + right.m11*left.m13;
    result.m10 = right.m8*left.m2 + right.m9*left.m6 + right.m10*left.m10 + right.m11*left.m14;
    result.m11 = right.m8*left.m3 + right.m9*left.m7 + right.m10*left.m11 + right.m11*left.m15;
    result.m12 = right.m12*left.m0 + right.m13*left.m4 + right.m14*left.m8 + right.m15*left.m12;
    result.m13 = right.m12*left.m1 + right.m13*left.m5 + right.m14*left.m9 + right.m15*left.m13;
    result.m14 = right.m12*left.m2 + right.m13*left.m6 + right.m14*left.m10 + right.m15*left.m14;
    result.m15 = right.m12*left.m3 + right.m13*left.m7 + right.m14*left.m11 + right.m15*left.m15;

    return result;
}

// Returns translation matrix
Matrix FMatrixTranslate(float x, float y, float z)
{
    Matrix result = { 1.0f, 0.0f, 0.0f, 0.0f, 
                      0.0f, 1.0f, 0.0f, 0.0f, 
                      0.0f, 0.0f, 1.0f, 0.0f, 
                      x, y, z, 1.0f };

    return result;
}

// Create rotation matrix from axis and angle provided in radians
Matrix FMatrixRotate(Vector3 axis, float angle)
{
    Matrix result;

    Matrix mat = FMatrixIdentity();
    float x = axis.x, y = axis.y, z = axis.z;
    float length = sqrt(x*x + y*y + z*z);

    if ((length != 1.0f) && (length != 0.0f))
    {
        length = 1.0f/length;
        x *= length;
        y *= length;
        z *= length;
    }

    float sinres = sinf(angle);
    float cosres = cosf(angle);
    float t = 1.0f - cosres;

    // Cache some matrix values (speed optimization)
    float a00 = mat.m0, a01 = mat.m1, a02 = mat.m2, a03 = mat.m3;
    float a10 = mat.m4, a11 = mat.m5, a12 = mat.m6, a13 = mat.m7;
    float a20 = mat.m8, a21 = mat.m9, a22 = mat.m10, a23 = mat.m11;

    // Construct the elements of the rotation matrix
    float b00 = x*x*t + cosres, b01 = y*x*t + z*sinres, b02 = z*x*t - y*sinres;
    float b10 = x*y*t - z*sinres, b11 = y*y*t + cosres, b12 = z*y*t + x*sinres;
    float b20 = x*z*t + y*sinres, b21 = y*z*t - x*sinres, b22 = z*z*t + cosres;

    // Perform rotation-specific matrix multiplication
    result.m0 = a00*b00 + a10*b01 + a20*b02;
    result.m1 = a01*b00 + a11*b01 + a21*b02;
    result.m2 = a02*b00 + a12*b01 + a22*b02;
    result.m3 = a03*b00 + a13*b01 + a23*b02;
    result.m4 = a00*b10 + a10*b11 + a20*b12;
    result.m5 = a01*b10 + a11*b11 + a21*b12;
    result.m6 = a02*b10 + a12*b11 + a22*b12;
    result.m7 = a03*b10 + a13*b11 + a23*b12;
    result.m8 = a00*b20 + a10*b21 + a20*b22;
    result.m9 = a01*b20 + a11*b21 + a21*b22;
    result.m10 = a02*b20 + a12*b21 + a22*b22;
    result.m11 = a03*b20 + a13*b21 + a23*b22;
    result.m12 = mat.m12;
    result.m13 = mat.m13;
    result.m14 = mat.m14;
    result.m15 = mat.m15;

    return result;
}

// Returns scaling matrix
Matrix FMatrixScale(float x, float y, float z)
{
    Matrix result = { x, 0.0f, 0.0f, 0.0f, 
                      0.0f, y, 0.0f, 0.0f, 
                      0.0f, 0.0f, z, 0.0f, 
                      0.0f, 0.0f, 0.0f, 1.0f };

    return result;
}

// Transposes provided matrix
void FMatrixTranspose(Matrix *mat)
{
    Matrix temp;

    temp.m0 = mat->m0;
    temp.m1 = mat->m4;
    temp.m2 = mat->m8;
    temp.m3 = mat->m12;
    temp.m4 = mat->m1;
    temp.m5 = mat->m5;
    temp.m6 = mat->m9;
    temp.m7 = mat->m13;
    temp.m8 = mat->m2;
    temp.m9 = mat->m6;
    temp.m10 = mat->m10;
    temp.m11 = mat->m14;
    temp.m12 = mat->m3;
    temp.m13 = mat->m7;
    temp.m14 = mat->m11;
    temp.m15 = mat->m15;

    *mat = temp;
}

// Transform a quaternion given a transformation matrix
void FMultiplyMatrixVector(Vector4 *v, Matrix mat)
{
    float x = v->x;
    float y = v->y;
    float z = v->z;
    float w = v->w;

    v->x = mat.m0*x + mat.m4*y + mat.m8*z + mat.m12*w;
    v->y = mat.m1*x + mat.m5*y + mat.m9*z + mat.m13*w;
    v->z = mat.m2*x + mat.m6*y + mat.m10*z + mat.m14*w;
    v->w = mat.m3*x + mat.m7*y + mat.m11*z + mat.m15*w;
}

// Returns the cosine value of a radian angle
float FCos(float value)
{    
    return (float)cos(value);
}

// Returns the sine value of a radian angle
float FSin(float value)
{    
    return (float)sin(value);
}

// Returns the tangent value of a radian angle
float FTan(float value)
{    
    return (float)tan(value);
}

// Returns a value to the power of an exponent
float FPower(float value, float exp)
{    
    return (float)pow(value, exp);
}

// Returns the square root of the input value
float FSquareRoot(float value)
{    
    return (float)sqrt(value);
}

// Returns a value rounded based on the samples
float FPosterize(float value, float samples)
{
    float output = value*samples;

    output = (float)floor(output);
    output /= samples;

    return output;
}

// Returns a value clamped by a min and max values
float FClamp(float value, float min, float max)
{
    float output = value;

    if (output < min) output = min;
    else if (output > max) output = max;

    return output;
}

// Returns a truncated value of a value
float FTrunc(float value)
{
    float output = value;

    int truncated = (int)output;
    output = (float)truncated;

    return output;
}

// Returns a rounded value of a value
float FRound(float value)
{
    float output = fabs(value);

    int truncated = (int)output;
    float decimals = output - (float)truncated;

    output = ((decimals > 0.5f) ? (output - decimals + 1.0f) : (output - decimals));
    if (value < 0.0f) output *= -1;

    return output;
}

// Returns a rounded up to the nearest integer of a value
float FCeil(float value)
{
    float output = value;

    int truncated = (int)output;
    if (output != (float)truncated) output = (float)((output >= 0) ? (truncated + 1) : truncated);

    return output;
}

// Returns the interpolation between two values
float FLerp(float valueA, float valueB, float time)
{
    return (float)(valueA + (valueB - valueA)*time);
}

// Returns the interpolate of a value in a range
float FSmoothStep(float min, float max, float value)
{
    float output = 0.0f;

    if (min <= max) output = FClamp((value/(max - min) - min), 0.0f, 1.0f);
    else output = FClamp((value/(min - max) - max), 0.0f, 1.0f);

    return output;
}

// Returns an ease linear value between two parameters
float FEaseLinear(float t, float b, float c, float d)
{ 
    return (float)(c*t/d + b); 
}

// Returns an ease quadratic in-out value between two parameters
float FEaseInOutQuad(float t, float b, float c, float d)
{
    float output = 0.0f;

	t /= d/2;
	if (t < 1) output = (float)(c/2*t*t + b);
    else
    {
        t--;
        output = (float)(-c/2*(t*(t-2) - 1) + b);
    }

	return output;
}

// Sends a float conversion value of a string to an initialized float pointer
void FStringToFloat(float *pointer, const char *string)
{
    *pointer = (float)atof(string);
}

// Sends formatted output to an initialized string pointer
void FFloatToString(char *buffer, float value)
{
    sprintf(buffer, "%.3f", value);
}

// Returns 1 if a specific string is found in a text file
int FSearch(char *filename, char *string)
{
	FILE *file;
	int found = 0;
	char temp[512];

	if ((file = fopen(filename, "r")) == NULL) TraceLogFNode(0, "(1) error when trying to open a file to search in");

	while (fgets(temp, 512, file) != NULL) 
    {
		if ((strstr(temp, string)) != NULL)
        {
            found = 1;
            break;
        }
	}

    // Close file if needed
	if (file) fclose(file);

   	return found;
}