#define RAYGUI_IMPLEMENTATION
#include <raylib.h>
#include "raygui.h"
#include "novaphysics/novaphysics.h"
#include <rlgl.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define PLATFORM_WEB

#ifdef PLATFORM_WEB
#include <emscripten.h>
#endif

#define MENU_HEIGHT 100
#define DNA_SIZE 450 
#define ROCKET_SIZE 25
#define NUM_ROCKETS 100 
#define ROCKET_WIDTH 20
#define ROCKET_HEIGHT 20
#define SCREEN_WIDTH 600
#define SCREEN_HEIGHT 600
#define ROCKETBITS 0x000000002
#define OBSTACLEBITS 0x00000004
#define NUM_SUPER_CLONES 5
#define NUM_SURE_MUTATIONS 0
#define PERCENT_MUTATED 0.10f
#define MUTATION_SIZE 9000
#define MAX_FORCE 50000
#define LATERAL_MAX 50000
#define DEBUG 0
#define TARGET_SIZE 12
#define ROCKET_X 100
#define ROCKET_Y SCREEN_HEIGHT - GROUND_HEIGHT - (ROCKET_HEIGHT / 2)
#define GROUND_HEIGHT 20

struct Target
{
    double x;
    double y;
};
struct Rocket
{
    nvShape *shape;
    nvRigidBody *bod;

    double dna[DNA_SIZE * 2];
    int width;
    int height;
    int lifecount;
    double fitness;
};

static struct Rocket **allRockets;
struct Target targ;
static int generation;
static int count;
static float speed;
static nvSpace *space;
static struct Rocket *tester;



double randNum(int min, int max)
{
    return rand() % (max - min + 1) + min;
}

// Copied from SO id 22186423
void gen_random_numbers(double *array, int len, int min, int max)
{
    for (int i = 0; i < len; i++)
    {
        if (i % 2 != 0)
        {
            array[i] = randNum(min, max);
        }
        else
        {
            array[i] = randNum(-1 * LATERAL_MAX, LATERAL_MAX);
        }
    }
}
double mutateValue(double init, int mutSize, int n)
{

    // Lateral Force then
    if (n % 2 == 0)
    {
        return (double)((int)(init + randNum(-1 * mutSize, mutSize)) % LATERAL_MAX);
    }

    else
    {
        return (double)((int)(init + randNum((int)(-1.0f * (float)mutSize), mutSize)) % MAX_FORCE);
    }
}

void mutateDNA(double *arr1, double *arr2, double *arr3, int len)
{

    int midpoint = rand() % len;

    for (int i = 0; i < len; i++)
    {

        if (midpoint < i)
        {
            arr1[i] = arr2[i];
        }
        if (rand() % 300 == 0)
        {
            if (i % 2 == 0)
            {
                arr1[i] = randNum(-1 * LATERAL_MAX, LATERAL_MAX);
            }
            else
            {
                arr1[i] = randNum(-1 * MAX_FORCE, MAX_FORCE);
            }
        }
        else
        {
            arr1[i] = arr3[i];
        }
    }
}

int compareFitness(const void *rocket1, const void *rocket2)
{

    const struct Rocket *rock1 = *(const struct Rocket **)rocket1;
    const struct Rocket *rock2 = *(const struct Rocket **)rocket2;

    return (rock2->fitness - rock1->fitness < 0.f ? -1 : 1);
}

// Sort each rocket in array by fitness
// Top performer can be a clone.
// Next x percent all have dna mutated from clone
// remaining bits get new dna.
void breedNewRockets(struct Rocket **temp)
{

    qsort(allRockets, NUM_ROCKETS, sizeof(struct Rocket *), compareFitness);
    struct Rocket *rocks[NUM_ROCKETS];

    for (int i = 0; i < NUM_ROCKETS; i++)
    {

        rocks[i] = allRockets[i];
    }

    int badRocketIndex = (((NUM_ROCKETS - 1) * 3) / 4);
    for (int i = 1; i < NUM_ROCKETS; i++)
    {

        allRockets[i]->fitness = allRockets[0]->fitness / allRockets[i]->fitness;

        if (allRockets[i]->fitness < .85f)
        {
            badRocketIndex = i;
            break;
        }
    }
    allRockets[0]->fitness = 1.0f;

    for (int i = NUM_SUPER_CLONES; i < NUM_ROCKETS - NUM_SURE_MUTATIONS; i++)
    {

        mutateDNA(allRockets[i]->dna, allRockets[rand() % NUM_SUPER_CLONES]->dna, allRockets[rand() % ((i % badRocketIndex) + 1)]->dna, DNA_SIZE * 2);
        // allRockets[i]->dna = allRockets[0]->dna;
    }

    for (int i = NUM_ROCKETS - NUM_SURE_MUTATIONS; i < NUM_ROCKETS; i++)
    {

        gen_random_numbers(allRockets[i]->dna, DNA_SIZE * 2, -MAX_FORCE, MAX_FORCE);
    }
}

void rocketUpdate()
{

    for (int i = 0; i < NUM_ROCKETS; i++)
    {

        nvVector2 pos = nvRigidBody_get_position(allRockets[i]->bod);

        float angle = nvRigidBody_get_angle(allRockets[i]->bod);
        float y = sin(angle);
        float x = cos(angle);

        allRockets[i]->fitness = 1.0f / (sqrt(pow(targ.x - pos.x + (x * (ROCKET_HEIGHT / 2)), 2) + pow(targ.y - pos.y + (y * (ROCKET_HEIGHT / 2)), 2)));

        double timeValue = (1.0f / allRockets[i]->lifecount) * allRockets[i]->fitness;
        allRockets[i]->fitness += timeValue;

        int lifecount = allRockets[i]->lifecount++;

        float torque = allRockets[i]->dna[lifecount * 2];

        nvVector2 force = {y * allRockets[i]->dna[(lifecount * 2) + 1], -1 * x * allRockets[i]->dna[(lifecount * 2) + 1]};

        nvRigidBody_apply_force(allRockets[i]->bod, force);
        nvRigidBody_apply_torque(allRockets[i]->bod, torque);

    }
}

void rocketDraw()
{

    for (int i = NUM_ROCKETS - 1; i >= 0; i--)
    {

        nvVector2 pos = nvRigidBody_get_position(allRockets[i]->bod);

        Color color = BLACK;
        if(i < NUM_SUPER_CLONES && generation > 1){
            
            color = PURPLE;

        }

        DrawRectanglePro((Rectangle){pos.x, pos.y, allRockets[i]->width, allRockets[i]->height}, (Vector2){allRockets[i]->width / 2, allRockets[i]->height / 2}, nvRigidBody_get_angle(allRockets[i]->bod), color);
    }
}
void drawTarget()
{



    DrawRectangle(targ.x - TARGET_SIZE , targ.y - TARGET_SIZE, TARGET_SIZE * 2, TARGET_SIZE * 2, RED);
    DrawRectangle(targ.x - (TARGET_SIZE / 2.f), targ.y - (TARGET_SIZE / 2.f), TARGET_SIZE, TARGET_SIZE, WHITE);
    DrawRectangle(targ.x - (TARGET_SIZE / 4.f), targ.y - (TARGET_SIZE / 4.f), TARGET_SIZE / 2.f, TARGET_SIZE / 2.f, RED);

}
void makeRocket(struct Rocket *rock, nvSpace *space, bool makeDNA)
{

    if (makeDNA)
    {
        gen_random_numbers(rock->dna, DNA_SIZE * 2, -MAX_FORCE / 4, MAX_FORCE);
    }
    nvRigidBodyInitializer body_init = nvRigidBodyInitializer_default;
    body_init.type = nvRigidBodyType_DYNAMIC;
    body_init.position = NV_VECTOR2(ROCKET_X, ROCKET_Y);
    body_init.material = (nvMaterial){.density=1.0, .restitution=0.0, .friction=0.1};
    rock->bod = nvRigidBody_new(body_init);
    nvRigidBody_set_collision_group(rock->bod, 1);
    rock->shape = nvRectShape_new(ROCKET_WIDTH, ROCKET_HEIGHT, nvVector2_zero);

    nvRigidBody_add_shape(rock->bod, rock->shape);
    nvSpace_add_rigidbody(space, rock->bod);
    rock->width = ROCKET_WIDTH;
    rock->height = ROCKET_HEIGHT;
    rock->lifecount = 0;
    rock->fitness = 0.f;
}

void resetRockets()
{
    for (int i = 0; i < NUM_ROCKETS; i++)
    {

        allRockets[i]->fitness = 0;
        allRockets[i]->lifecount = 0;
        nvRigidBody_set_angle(allRockets[i]->bod, 0.0f);
        nvRigidBody_set_linear_velocity(allRockets[i]->bod, nvVector2_zero);
        nvRigidBody_set_angular_velocity(allRockets[i]->bod, 0);
        nvRigidBody_set_position(allRockets[i]->bod, (nvVector2) {ROCKET_X, ROCKET_Y});
    }
}
void right(struct Rocket *rock)
{


    
    nvRigidBody_apply_torque(rock->bod, 50000);
}
void left(struct Rocket *rock)
{


     nvRigidBody_apply_torque(rock->bod, -50000);
}

void up(struct Rocket *rock)
{


    float angle = nvRigidBody_get_angle(rock->bod);

    float y = sin(angle);
    float x = cos(angle);

    nvRigidBody_apply_force(rock->bod, (nvVector2){y * 10000, x * -10000});
}
void drawTester(struct Rocket *rock)
{

    nvVector2 pos = nvRigidBody_get_position(rock->bod);

    DrawRectanglePro((Rectangle){pos.x, pos.y, rock->width, rock->height}, (Vector2){rock->width / 2, rock->height / 2}, nvRigidBody_get_angle(rock->bod) * (180.f / PI), GREEN);
}

void setRandomTarget(struct Target *targ){

    targ->x = (rand() % (SCREEN_WIDTH - (2 * TARGET_SIZE))) + TARGET_SIZE;

    targ->y = (rand() % (SCREEN_HEIGHT - GROUND_HEIGHT - MENU_HEIGHT - TARGET_SIZE)) + MENU_HEIGHT + TARGET_SIZE ;


}

void updateDrawFrame(){
        if (IsKeyDown(KEY_RIGHT))
            right(tester);
        if (IsKeyDown(KEY_LEFT))
            left(tester);
        if (IsKeyDown(KEY_UP))
            up(tester);

        if (count + 1 < DNA_SIZE)
        {
            nvSpace_step(space, 1.0f / 60.0f);
            rocketUpdate();

            if (count % (int)speed == 0)
            {

                BeginDrawing();
                ClearBackground(RAYWHITE);
                drawTester(tester);
                rocketDraw();
                DrawRectangle(0, 0, SCREEN_WIDTH, MENU_HEIGHT, GRAY);
                char buffer[50];  
                snprintf(buffer, sizeof(buffer), "%s%d", "Generation: ", generation);
                GuiSetStyle(DEFAULT, TEXT_SIZE, 20);
                GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, 0x33FFFF);

                #ifndef PLATFORM_WEB
                GuiSliderBar((Rectangle){(SCREEN_WIDTH / 2) + 150, 20, 100, 40}, "Speed", TextFormat(" %.0fx", speed), &speed, 1, 4);
                #endif

                if(GuiButton((Rectangle){200, 20, 200, 70}, "Change Target")){

                    setRandomTarget(&targ);
                    resetRockets();
                    generation = 1;
                    count = -1;

                }
                DrawText(buffer, 20, 30, 20, ORANGE);
                DrawRectangle(0, SCREEN_HEIGHT - GROUND_HEIGHT, SCREEN_WIDTH, GROUND_HEIGHT, GetColor(0x006400ff));
                drawTarget();
                EndDrawing();
            }
        }
        else
        {
            generation++;
            breedNewRockets(allRockets);
            resetRockets();
            count = -1;
        }
        count++;

}

int main()
{
    space = nvSpace_new();
    srand(time(NULL));
    // Trying to improve performance
    // Profiler says most time is spent in the hashmap 
    // implementation of nova physics.
    nvSpace_get_settings(space)->position_iterations = 1;
    nvSpace_get_settings(space)->substeps = 1;
    nvSpace_set_gravity(space, (nvVector2) {0.0f, 10.0f});

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Genetic Rockets");


    generation = 1;
    count = 0;
    speed = 1.0f;
    setRandomTarget(&targ);

    allRockets = malloc((NUM_ROCKETS * sizeof(struct Rocket *)) + 1);

    tester = malloc(sizeof(struct Rocket));
    makeRocket(tester, space, 1);

    for (int i = 0; i < NUM_ROCKETS; i++)
    {

        allRockets[i] = malloc(sizeof(struct Rocket));

        makeRocket(allRockets[i], space, 1);
    }
    nvRigidBody_set_position(tester->bod, (nvVector2){500, ROCKET_Y});
    nvRigidBodyInitializer body_init = nvRigidBodyInitializer_default;
    body_init.type = nvRigidBodyType_STATIC;
    body_init.position = NV_VECTOR2(SCREEN_WIDTH / 2, SCREEN_HEIGHT - (GROUND_HEIGHT / 2));
    body_init.material = nvMaterial_CONCRETE; 
    nvRigidBody *ground = nvRigidBody_new(body_init);

    nvRigidBody_add_shape(ground, nvRectShape_new(SCREEN_WIDTH, GROUND_HEIGHT, nvVector2_zero));
    nvSpace_add_rigidbody(space, ground);

    #ifdef PLATFORM_WEB
    emscripten_set_main_loop(updateDrawFrame, 120, 1);
    #else
    SetTargetFPS(60);

    while(!WindowShouldClose()){
        updateDrawFrame();


    }   
    #endif

    for (int i = 0; i < NUM_ROCKETS; i++)
    {
        free(allRockets[i]);
    }
    free(allRockets);
    nvSpace_free(space);
    CloseWindow();
    return 0;
}
