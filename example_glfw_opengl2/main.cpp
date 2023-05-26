#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl2.h"
#include <stdio.h>
#include <GLFW/glfw3.h>
#include <fstream>
#include <vector>
#include <iostream>
#include <string>
#include <random>

#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

struct flash_card
{
    std::string question{};
    std::string answer{};
};

auto get_question(std::string s) -> std::string
{
    int index = s.find("..|..");
    return s.substr(0, index - 1);
}

auto get_answer(std::string s) -> std::string
{
    int index = s.find("..|..");
    return s.substr(index + 5, s.size());
}

int main(int, char**)
{
    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Dear ImGui GLFW+OpenGL2 example", NULL, NULL);
    if (window == NULL)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO const& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsLight();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL2_Init();
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.54f, 0.66f, 0.60f, 0.86f);
    bool randome_window = false;
    std::string _question{};
    bool to_test_flash_cards = false;
    bool done_answer = false;
    bool done_question = false;
    bool _to_test_flash_cards = false;
    std::string __question{};
    std::string __answer{};
    bool open_input_window = false;
    bool window_open = true;

    while (!glfwWindowShouldClose(window) && window_open)
    {
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
        {
            ImGui::Begin("FlashCards++", &window_open);

            if (ImGui::Button("Test Me on flash cards"))
            {
                /*Test on flash cards using a file*/
                std::string line{};
                std::vector<flash_card> vec{};
                std::ifstream myfile("flashcards.txt");
                if (myfile.is_open())
                {
                    while (std::getline(myfile, line))
                    {
                        std::string first_part = get_question(line);
                        std::string second_part = get_answer(line);
                        vec.push_back(flash_card{first_part, second_part});
                    }
                    myfile.close();
                }

                std::random_device dev;
                std::mt19937 rng(dev());
                std::uniform_int_distribution<std::mt19937::result_type> dist6(0, vec.size() - 1); // distribution in range [1, vec.size()]
                auto random = dist6(rng);
                to_test_flash_cards = true;
                _question = vec[random].question;
            }
            if (ImGui::Button("Add another flash card"))
            {
                /*Add another flash card to the flash cards file*/
                open_input_window = true;
            }
            if (ImGui::Button("Show me the flash cards"))
            {
                std::string line{};
                std::vector<flash_card> vec{};
                std::ifstream myfile("flashcards.txt");
                if (myfile.is_open())
                {
                    while (std::getline(myfile, line))
                    {
                        std::string first_part = get_question(line);
                        std::string second_part = get_answer(line);
                        vec.push_back(flash_card{ first_part, second_part });
                    }
                    myfile.close();
                }

                std::random_device dev;
                std::mt19937 rng(dev());
                std::uniform_int_distribution<std::mt19937::result_type> dist6(0, vec.size() - 1); // distribution in range [1, vec.size()]
                auto random = dist6(rng);
                _to_test_flash_cards = true;
                __question = vec[random].question;
                __answer = vec[random].answer;
                /*Open a window that shows all of the flash cards and opens new windows for new flash cards*/
            }
            if (ImGui::Button("Quit"))
            {
                /*Exit all windows*/
                window_open = false;
            }

            //if (button < 10 && button >= 0)
            //{
            //    if (
            //        ImGui::Button("Click me for how many pizzas"))
            //    {
            //        button++;
            //    }
            //    else if (ImGui::Button("Click me to get rid of some pizzas"))
            //    {
            //        --button;
            //    }
            //}
            //else if(button >= 10)
            //{
            //    if (ImGui::Button("Click me to decrease some pizzas!"))
            //    {
            //        --button;
            //    }
            //}
            //else if (button <= 0)
            //{
            //    ImGui::NewLine();
            //    ImGui::Text("So your telling me you want -1 pizzas?\nThat can't be right!");
            //}
            //ImGui::NewLine();
            //ImGui::Text("You want %d pizzas", button);

            //ImGui::NewLine();

            //if (!exceeds_limit(button, 10))
            //{
            //    ImGui::Text("Hold on there buddy!\nYou have reached our maximum amount of pizzas!");
            //}
            //else
            //{
            //    ImGui::Text("You can order %d more pizzas at most!", 10 - button);
            //}

            //ImGui::NewLine();
            //if (ImGui::Button("Click here to reset"))
            //{
            //    button = 1;
            //}

            //ImGui::NewLine();
            //ImGui::Bullet();
            //ImGui::Text("Contact us on 07903873501");

            //ImGui::NewLine();
            //ImGui::Bullet();
            //ImGui::Text("Or contact us on email at rshepherdcpp@gmail.com");

            //static bool t_and_c = false;

            //ImGui::NewLine();
            //ImGui::Checkbox("Do you agree to the terms and conditions?", &t_and_c);

            //if (t_and_c)
            //{
            //    ImGui::Text("You don't know what you just agreed to.\n");
            //    ImGui::Text("You have agreed to give me all of your JavaScript code!");
            //}

            //ImGui::NewLine();
            //if (ImGui::ColorButton("colourful", clear_color))
            //{
            //    ImGui::Text("You just chose to open a new window!\n");
            //    ImGui::Text("Opening Now");

            //    randome_window = true;
            //}



            //ImGui::NewLine();
            //ImVec4 magical_colour = ImVec4(0.102f, 0.96f, 0.72f, 0.15f);

            //if (ImGui::ColorButton("Click here to stop", magical_colour))
            //{
            //    break;
            //}
            //ImGui::Text("Click Here To Stop");

            ImGui::End();
        }
        if (open_input_window)
        {
            char question[256] = {};
            char answer[256] = {};
            {
                if (ImGui::Button("You chose to add another flash card"));
                ImGui::Begin("Input");
                char question[256] = {};
                if (!done_question)
                {
                    ImGui::InputText("Enter your question ", question, 256);
                    done_question = true;
                }

                if (!done_answer)
                {
                    ImGui::InputText("Enter your answer: ", answer, 256);
                    done_question = true;
                }
                if (ImGui::Button("Finish"))
                {
                    std::ofstream file("flashcards.txt", std::ios::app);
                    file << question << "..|.." << answer;
                    open_input_window = false;
                }
                ImGui::End();
            }
        }

        if (to_test_flash_cards)
        {
            {
                ImGui::SetNextWindowSize(ImVec2(600, 200));
                ImGui::Begin(_question.c_str());
                ImGui::End();
            };
        }

        if (_to_test_flash_cards)
        {
            {
                ImGui::SetNextWindowSize(ImVec2(600, 200));
                ImGui::Begin(__question.c_str());
                std::string value = (std::string{ __question.c_str() } + std::string{ " : " } + std::string{ __answer.c_str() });
                if (ImGui::Button(value.c_str()))
                {
                }
                ImGui::End();
            };
        }

            // Rendering
            ImGui::Render();
            int display_w, display_h;
            glfwGetFramebufferSize(window, &display_w, &display_h);
            glViewport(0, 0, display_w, display_h);
            glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
            glClear(GL_COLOR_BUFFER_BIT);

            ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
            glfwMakeContextCurrent(window);
            glfwSwapBuffers(window);
        }

        // Cleanup
        ImGui_ImplOpenGL2_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        glfwDestroyWindow(window);
        glfwTerminate();

        return 0;
    }
