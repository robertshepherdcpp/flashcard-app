// Dear ImGui: standalone example application for GLFW + OpenGL2, using legacy fixed pipeline
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

// **DO NOT USE THIS CODE IF YOUR CODE/ENGINE IS USING MODERN OPENGL (SHADERS, VBO, VAO, etc.)**
// **Prefer using the code in the example_glfw_opengl2/ folder**
// See imgui_impl_glfw.cpp for details.

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl2.h"
#include <stdio.h>
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif
#include <GLFW/glfw3.h>
#include <string> // std::string
#include <vector> // std::vector
#include <fstream> // file manipulation
#include <random> // for random things.
#include <asio.hpp>
#include <vector> // std::vector
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define GL_CLAMP_TO_EDGE 0x812F
#include <iostream>

// Simple helper function to load an image into a OpenGL texture with common settings
bool LoadTextureFromFile(const char* filename, GLuint* out_texture, int* out_width, int* out_height)
{
    // Load from file
    int image_width = 0;
    int image_height = 0;
    int random_number = 0;
    unsigned char* image_data = stbi_load(filename, &image_width, &image_height, NULL, 4);
    if (image_data == NULL)
        return false;

    // Create a OpenGL texture identifier
    GLuint image_texture;
    glGenTextures(1, &image_texture);
    glBindTexture(GL_TEXTURE_2D, image_texture);

    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same

    // Upload pixels into texture
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    stbi_image_free(image_data);

    *out_texture = image_texture;
    *out_width = image_width;
    *out_height = image_height;

    return true;
}

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

using asio::ip::tcp;

std::string get_website_content(const std::string& host, const std::string& path) {
    std::string result;

    try {
        asio::io_context io_context;

        // Resolve the host name and service (http in this case)
        tcp::resolver resolver(io_context);
        tcp::resolver::results_type endpoints = resolver.resolve(host, "http");

        // Establish a connection to the server
        tcp::socket socket(io_context);
        asio::connect(socket, endpoints);

        // Prepare the HTTP GET request
        asio::streambuf request;
        std::ostream request_stream(&request);
        request_stream << "GET " << path << " HTTP/1.1\r\n";
        request_stream << "Host: " << host << "\r\n";
        request_stream << "Connection: close\r\n\r\n";

        // Send the HTTP request
        asio::write(socket, request);

        // Read and process the response
        asio::streambuf response;
        asio::read_until(socket, response, "\r\n");

        // Check if the response was successful
        std::istream response_stream(&response);
        std::string http_version;
        response_stream >> http_version;
        unsigned int status_code;
        response_stream >> status_code;

        if (http_version.substr(0, 5) != "HTTP/") {
            std::cerr << "Invalid response\n";
            return result;
        }

        if (status_code != 200) {
            std::cerr << "Request failed with status code " << status_code << "\n";
            return result;
        }

        // Read the rest of the response
        asio::read(socket, response, asio::transfer_all());

        // Extract the content from the response
        std::istreambuf_iterator<char> iterator(&response);
        std::istreambuf_iterator<char> end;
        result.assign(iterator, end);

    }
    catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return result;
}

namespace exceptions
{
    struct Exception
    {
        std::string what{};
    };

    // Just inherit the properties
    struct BadArguementPassed : Exception
    {
    };
}; // namespace exceptions

struct flashcard
{
    std::string question{};
    std::string answer{};
};

auto get_question(std::string& s) -> std::string
{
    auto index = s.find("||");
    return s.substr(0, index - 1);
}

auto get_answer(std::string& s) -> std::string
{
    auto index = s.find("||");
    return s.substr(index + 3, s.size());
}

auto find_in_vec(std::string& x, std::vector<std::string>& vec)
{
    for (int i = 0; i < vec.size(); i++)
    {
        if (vec[i] == x)
        {
            return i;
        }
    }
    return -1;
}

auto get_random_number(auto mod)
{
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist(1, mod);
    return dist(rng);
}

// this container is to help simplify the code
struct optional_bool
{
    optional_bool(bool first_, bool second_)
        : m_first{ first_ }, m_second{ second_ }
    {
    }

    bool m_first = true;
    bool m_second = false;

    // to help express intend more clearly
    std::string m_first_name{};
    std::string m_second_name{};

    auto set_bools(bool a, bool b)
    {
        m_first = a;
        m_second = b;
    }

    auto set_names(std::string first_name_, std::string second_name_) -> void
    {
        m_first_name = first_name_;
        m_second_name = second_name_;
    }

    auto get_by_name(std::string name_) -> bool
    {
        if (name_ == m_first_name)
        {
            return m_first;
        }
        if (name_ == m_second_name)
        {
            return m_second;
        }
        else
        {
            throw exceptions::BadArguementPassed{"The name passed to get_by_name, was not set by set_names"};
        }
    }

    auto flip()
    {
        if (m_first)
        {
            m_first = false;
            m_second = true;
        }
        else if (m_second)
        {
            m_first = true;
            m_second = false;
        }
    }
};

struct user
{
    std::string username{};
    std::string password{};
    int score{};

    user(std::string x, std::string y, int z)
        : username{ x }, password{ y }, score{ z }
    {
    }

    user() {} // do nothing

    auto operator=(user x_user)
        //: username{ x_user.username }, password{x_user.password}, score{x_user.score}
    {
        username = x_user.username;
        password = x_user.password;
        score = x_user.score;
    }
};

int colour_background_one = 63;
int colour_background_two = 200;
int colour_background_three = 6;

// Main code
int main(int, char**)
{
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Dear ImGui GLFW+OpenGL2 example", nullptr, nullptr);
    if (window == nullptr)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiStyle& style = ImGui::GetStyle();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    //style.Colors[ImGuiCol_Text] = ImColor(2, 2, 2);
    //style.Colors[ImGuiCol_WindowBg] = ImColor(colour_background_one, colour_background_two, colour_background_three);

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL2_Init();

    // Our state
    bool get_flashcards_from_web = false;
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    bool show_the_flashcards_window = false;
    float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
    int current_flashcard_position = 0;
    bool give_error_message_answer_question = false;
    optional_bool IsQuestionAnswer{true, false};
    IsQuestionAnswer.set_names("IsQuestion", "IsAnswer");
    int point_count = 0;
    bool celebrate_thousand_points = false;
    bool login_window = false;
    bool user_logged_in = false;
    optional_bool LaunchFlashCard{false, false};
    bool give_praise_on_getting_flashcard_right = false;
    LaunchFlashCard.set_names("launch_flash_card", "launch_flash_card_other");
    bool login_page_open = true;
    bool login_window_option = false;
    int random_number = 0;
    bool change_colour_background = false;
    user current_user{};
    bool test_on_flash_cards = false;
    std::vector<std::string> usernames{};
    std::vector<std::string> passwords{};
    std::vector<user> users = [&]()
    {
        std::string line{};
        std::vector<user> vec{};
        std::ifstream myfile("users.txt");
        if (myfile.is_open())
        {
            while (std::getline(myfile, line))
            {
                std::string username = [&]()
                {
                    int index_of_chevrons_into = line.find("<<");
                    usernames.push_back(std::string{ line.substr(0, index_of_chevrons_into - 1) });
                    return std::string{ line.substr(0, index_of_chevrons_into - 1) };
                }();
                std::string password = [&]()
                {
                    int index_of_chevrons_into = line.find("<<");
                    int index_of_chevrons_outo = line.find(">>");
                    passwords.push_back(std::string{ line.substr(index_of_chevrons_into + 3, index_of_chevrons_outo - 1) });
                    return std::string{line.substr(index_of_chevrons_into + 3, index_of_chevrons_outo - 1)};
                }();
                int score = [=]()
                {
                    int index_of_chevrons_outo = line.find(">>");
                    return std::stoi(std::string{line.substr(index_of_chevrons_outo + 3, line.size())});
                }();
                vec.push_back(user{ username, password, score });
            }
            myfile.close();
        }
        return vec;
    }();
    std::vector<flashcard> flashcards = [](std::string s)
    {
        std::string line{};
        std::vector<flashcard> vec{};
        std::ifstream myfile(s.c_str());
        if (myfile.is_open())
        {
            while (std::getline(myfile, line))
            {
                std::string first_part = get_question(line);
                std::string second_part = get_answer(line);
                vec.push_back(flashcard{ first_part, second_part });
            }
            myfile.close();
        }
        return vec;
    }("flashcards.txt");
    std::vector<int> flashcard_count(flashcards.size() + 1);

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
        {
            ImGui::Begin("FlashCards App");
            if (!user_logged_in)
            {
                if (ImGui::Button("Login"))
                {
                    login_window_option = true;
                }
            }
            if (user_logged_in)
            {
                if (ImGui::Button(current_user.username.c_str()))
                {
                    // do nothing just, it is just to show their username
                }
                ImGui::NewLine();
                ImGui::NewLine();
                if (ImGui::Button("Show me the flashcards"))
                {
                    show_the_flashcards_window = true;
                }
                ImGui::NewLine();
                if (ImGui::Button("Change colour background"))
                {
                    change_colour_background = true;
                }
                //ImGuiIO& io = ImGui::GetIO();
                //ImTextureID my_tex_id = io.Fonts->TexID;
                //float my_tex_w = (float)io.Fonts->TexWidth;
                //float my_tex_h = (float)io.Fonts->TexHeight;
                {
                    //static bool use_text_color_for_tint = false;
                    //ImGui::Checkbox("Use Text Color for Tint", &use_text_color_for_tint);
                    //ImGui::Text("%.0fx%.0f", my_tex_w, my_tex_h);
                    //ImVec2 pos = ImGui::GetCursorScreenPos();
                    //ImVec2 uv_min = ImVec2(0.0f, 0.0f);                 // Top-left
                    //ImVec2 uv_max = ImVec2(1.0f, 1.0f);                 // Lower-right
                    //ImVec4 tint_col = use_text_color_for_tint ? ImGui::GetStyleColorVec4(ImGuiCol_Text) : ImVec4(1.0f, 1.0f, 1.0f, 1.0f);   // No tint
                    //ImVec4 border_col = ImGui::GetStyleColorVec4(ImGuiCol_Border);
                    //ImGui::Image(my_tex_id, ImVec2(my_tex_w, my_tex_h), uv_min, uv_max, tint_col, border_col);
                    
                    //int my_image_width = 0;
                    //int my_image_height = 0;
                    //GLuint my_image_texture = 0;
                    //bool ret = LoadTextureFromFile("flashcard.png", &my_image_texture, &my_image_width, &my_image_height);
                    //IM_ASSERT(ret);
                    //ImGui::Image((void*)(intptr_t)my_image_texture, ImVec2(my_image_width, my_image_height));
                }
                /*
                if (ImGui::Button("Add a flash card"))
                {
                    /*
                    ImGui::Text("Check your Console for instructions");
                    std::string question{};
                    std::cout << "Question: ";
                    std::cin >> question;
                    std::string answer{};
                    std::cout << "Answer: ";
                    std::cin >> answer;
                    flashcards.push_back(flashcard{ question, answer });
                    // file handling.
                    std::ofstream file;
                    file.open("flashcards.txt", std::ios::out | std::ios::app);
                    if (file.fail())
                        throw std::ios_base::failure(std::strerror(errno));
                    file.exceptions(file.exceptions() | std::ios::failbit | std::ifstream::badbit);
                    file << question << " || " << answer << std::endl;
                    file.close();
                }
                */
                ImGui::NewLine();
                ImGui::Text("Your total point count: %d", point_count);
            }
            ImGui::End();
        }

        if (login_window)
        {
            ImGui::Begin("Login Page", &login_page_open);
            std::string s_username{};
            std::string s_password{};
            bool finished_username = false;
            bool finished_password = true;
            static char username_[128] = {};
            static char password_[128] = {};
            ImGui::InputText("Enter your username: ", username_, 128);
            ImGui::InputText("Enter your password: ", password_, 128);
            if (ImGui::Button("Finished username"))
            {
                s_username = username_;
            }
            if (ImGui::Button("Finished password"))
            {
                s_password = password_;
            }
            std::string username = s_username.substr(0, s_username.find('.'));
            std::string password = s_password.substr(0, s_password.find('.'));

            if (find_in_vec(username, usernames) != -1)
            {
                int index = find_in_vec(username, usernames);
                if (passwords[index] == password)
                {
                    current_user = users[index];
                }
                else
                {
                    ImGui::Text("Sorry you entered a wrong user name or password");
                }
            }
            if (ImGui::Button("Continue without login"))
            {
                login_window = false;
                user_logged_in = true;
            }
            ImGui::End();
        }

        if (login_window_option)
        {
            random_number = get_random_number(flashcards.size() - 1);
            ImGui::Begin("Login page");
            ImGui::Text("Who are you?");
            ImGui::NewLine();
            for (int i = 0; i < usernames.size(); i++)
            {
                if (ImGui::Button(usernames[i].c_str()))
                {
                    current_user = users[i];
                    point_count = users[i].score;
                    user_logged_in = true;
                    login_window_option = false;
                }
            }

            ImGui::End();
        }

        if (show_the_flashcards_window)
        {
            ImGui::Begin("FlashCard manager", &show_another_window);
            if (ImGui::ArrowButton("##left", ImGuiDir_Left))
            {
                flashcard_count[current_flashcard_position] += 1;
                // FIXME make this actually visible
                if (ImGui::IsItemHovered())
                {
                    ImGui::SetTooltip("Click me to get to the flashcard before");
                }

                if ((point_count < 1000) && ((point_count + 10) >= 1000))
                {
                    celebrate_thousand_points = true;
                }
                point_count += 10;
                IsQuestionAnswer.set_bools(true, false);
                LaunchFlashCard.m_first = false;
                LaunchFlashCard.m_second = true;
                if (current_flashcard_position == 0)
                {
                    // do nothing
                    current_flashcard_position += 1;
                }
                else
                {
                    current_flashcard_position -= 1;
                    LaunchFlashCard.m_first = true;
                }
            }
            ImGui::SameLine(0.0f, spacing);
            if (ImGui::ArrowButton("##right", ImGuiDir_Right))
            {
                // FIXME make the item actually visible
                flashcard_count[current_flashcard_position + 1] += 1;
                if (ImGui::IsItemHovered())
                {
                    ImGui::SetTooltip("Click me to get to the flashcard after");
                }

                if ((point_count < 1000) && ((point_count + 10) >= 1000))
                {
                    celebrate_thousand_points = true;
                }
                point_count += 10;
                IsQuestionAnswer.flip();
                LaunchFlashCard.m_first = true;
                LaunchFlashCard.m_second = false;
                if (current_flashcard_position == flashcards.size() - 1)
                {
                    // nothing because we cant
                    current_flashcard_position -= 1;
                }
                else
                {
                    current_flashcard_position += 1;
                    LaunchFlashCard.m_first = true;
                }
            }
            ImGui::SameLine(0.0f, spacing);
            if (ImGui::Button("Show"))
            {
                LaunchFlashCard.m_first = true;
            }
            ImGui::SameLine(0.0f, spacing);
            if (ImGui::Button("?"))
            {
                current_flashcard_position = get_random_number(flashcards.size() - 1);
                LaunchFlashCard.m_first = true;
            }
            ImGui::NewLine();
            ImGui::NewLine();
            if (ImGui::Button("Test"))
            {
                test_on_flash_cards = true;
            }

            ImGui::NewLine();
            ImGui::NewLine();
            ImGui::Text("You are currently on flashcard number %d", current_flashcard_position + 1);
            ImGui::NewLine();
            if (ImGui::Button("Add flashcards from the web"))
            {
                get_flashcards_from_web = true;
            }
            ImGui::End();
        }

        if (test_on_flash_cards)
        {
            ImGui::Begin("FlashCards Test");
            //int random_number = get_random_number(flashcards.size());
            // FIXME
            //int random_number = 2;
            if (random_number == 0)
            {
                random_number = 1;
            }
            if (random_number == flashcards.size() - 1)
            {
                random_number = flashcards.size() - 2;
            }
            ImGui::Text(flashcards[random_number].question.c_str());
            if (ImGui::Button(flashcards[random_number - 1].answer.c_str()))
            {
                give_error_message_answer_question = true;
                test_on_flash_cards = false;
                give_praise_on_getting_flashcard_right = true;
            }
            if (ImGui::Button(flashcards[random_number].answer.c_str()))
            {
                test_on_flash_cards = false;
                give_error_message_answer_question = false;
                give_praise_on_getting_flashcard_right = true;
                random_number = get_random_number(flashcards.size() - 1);
            }
            if (ImGui::Button(flashcards[random_number + 1].answer.c_str()))
            {
                give_error_message_answer_question = true;
                give_praise_on_getting_flashcard_right = false;
                test_on_flash_cards = false;
            }

            ImGui::End();
        }

        if (give_praise_on_getting_flashcard_right)
        {
            ImGui::Begin("Well done, you are a clever person!");
            ImGui::Text("Well, done! The answer that you selected was right!");
            ImGui::End();
        }

        if (change_colour_background)
        {
            ImGui::Begin("Colour Change");
            if (ImGui::ArrowButton("##up", ImGuiDir_Up))
            {
                if (colour_background_one < 255)
                {
                    colour_background_one += 1;
                    style.Colors[ImGuiCol_WindowBg] = ImColor(colour_background_one, colour_background_two, colour_background_three);
                }
            }
            ImGui::SameLine(0.0f, spacing);
            if (ImGui::ArrowButton("##down", ImGuiDir_Down))
            {
               if (colour_background_one > 0)
               {
                    colour_background_one -= 1;
                    style.Colors[ImGuiCol_WindowBg] = ImColor(colour_background_one, colour_background_two, colour_background_three);
               }
            }
            ImGui::SameLine(0.0f, spacing);
            ImGui::Text("%d", colour_background_one);
            ImGui::NewLine();
            if (ImGui::ArrowButton("##up", ImGuiDir_Up))
            {
                if (colour_background_two < 255)
                {
                    colour_background_two += 1;
                    style.Colors[ImGuiCol_WindowBg] = ImColor(colour_background_one, colour_background_two, colour_background_three);
                }
            }
            ImGui::SameLine(0.0f, spacing);
            if (ImGui::ArrowButton("##down", ImGuiDir_Down))
            {
                if (colour_background_two > 0)
                {
                    colour_background_two -= 1;
                    style.Colors[ImGuiCol_WindowBg] = ImColor(colour_background_one, colour_background_two, colour_background_three);
                }
            }
            ImGui::SameLine(0.0f, spacing);
            ImGui::Text("%d", colour_background_two);
            ImGui::NewLine();
            if (ImGui::ArrowButton("##up", ImGuiDir_Up))
            {
                if (colour_background_three < 255)
                {
                    colour_background_three += 1;
                    style.Colors[ImGuiCol_WindowBg] = ImColor(colour_background_one, colour_background_two, colour_background_three);
                }
            }
            ImGui::SameLine(0.0f, spacing);
            if (ImGui::ArrowButton("##down", ImGuiDir_Down))
            {
                if (colour_background_three > 0)
                {
                    colour_background_three -= 1;
                    style.Colors[ImGuiCol_WindowBg] = ImColor(colour_background_one, colour_background_two, colour_background_three);
                }
            }
            ImGui::SameLine(0.0f, spacing);
            ImGui::Text("%d", colour_background_three);
            ImGui::NewLine();
            ImGui::End();
        }

        if (get_flashcards_from_web)
        {
            ImGui::Begin("Get flashcards from the web");
            static char str0[128] = "Hello, world!";
            ImGui::InputText("input text", str0, IM_ARRAYSIZE(str0));
            std::string input{str0};
            ImGui::Text(input.c_str());
            std::string s = get_website_content(input, "/");
            ImGui::End();
        }

        if (celebrate_thousand_points)
        {
            ImGui::Begin("Well done!");
            ImGui::Text("You reached one thousand points! Well done");
            ImGui::Text("Keep the hard work up! It will pay off!");
            ImGui::NewLine();
            if (ImGui::Button("QUIT"))
            {
                celebrate_thousand_points = false;
            }
            ImGui::End();
        }

        if (give_error_message_answer_question)
        {
            ImGui::Begin("You got the question wrong :(");
            ImGui::Text("Sorry but the answer you selected was not true. Dont worry just try again :)");
            ImGui::End();
        }

        if (LaunchFlashCard.m_first)
        {
            //IsQuestionAnswer.set_bools(true, false);
            ImGui::Begin("FlashCard");
            flashcard current_flashcard = flashcards[current_flashcard_position];
            ImGui::Text(current_flashcard.question.c_str());
            //flashcard_count[current_flashcard_position] += 1;
            if (ImGui::Button("FLIP"))
            {
                if ((point_count < 1000) && ((point_count + 5) >= 1000))
                {
                    celebrate_thousand_points = true;
                }
                point_count += 5;
                IsQuestionAnswer.flip();
                LaunchFlashCard.set_bools(false, true);
            }
            ImGui::NewLine();
            if (ImGui::Button("QUIT"))
            {
                LaunchFlashCard.m_first = false;
            }
            ImGui::Text("You have seen this flashcard %d times", flashcard_count[current_flashcard_position]);
            ImGui::End();
        }

        if (LaunchFlashCard.m_second)
        {
            ImGui::Begin("FlashCard");
            flashcard current_flashcard = flashcards[current_flashcard_position];
            ImGui::Text(current_flashcard.answer.c_str());
            //flashcard_count[current_flashcard_position] += 1;
            if (ImGui::Button("FLIP"))
            {
                if ((point_count < 1000) && ((point_count + 5) >= 1000))
                {
                    celebrate_thousand_points = true;
                }
                point_count += 5;
                IsQuestionAnswer.flip();
                LaunchFlashCard.set_bools(true, false);
            }
            ImGui::NewLine();
            if (ImGui::Button("QUIT"))
            {
                LaunchFlashCard.m_first = false;
            }
            ImGui::Text("You have seen this flashcard %d times", flashcard_count[current_flashcard_position]);
            ImGui::End();
        }

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);

        // If you are using this code with non-legacy OpenGL header/contexts (which you should not, prefer using imgui_impl_opengl3.cpp!!),
        // you may need to backup/reset/restore other state, e.g. for current shader using the commented lines below.
        //GLint last_program;
        //glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
        //glUseProgram(0);
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
        //glUseProgram(last_program);

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
