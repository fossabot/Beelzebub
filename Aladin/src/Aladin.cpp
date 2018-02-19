//#define GLFW_INCLUDE_GLEXT
#define ALADIN_IMPLEMENTATION

#include <GLFW\glfw3.h>
#include <Aladin.h>
#include <AladinRender.h>

GLFWwindow* Window;
int Width, FramebufferWidth;
int Height, FramebufferHeight;
double Time = 0.0f;
bool MouseJustPressed[3] = { false, false, false };

int main(int argc, const char* argv[]) {
	AladinSetDPIAware();

	if (!glfwInit()) {
		printf("Could not initialize glfw");
		return -1;
	}

	//glfwGetmonitor

	// TODO: Fetch desktop resolution and create something less gay
	Width = 1200;
	Height = 800;

	Window = glfwCreateWindow(Width, Height, "Aladin", NULL, NULL);
	if (!Window) {
		printf("Could not create glfw window");
		glfwTerminate();
		return -1;
	}
	//glfwMaximizeWindow(Window);
	glfwMakeContextCurrent(Window);

	alInit();
	InitGUI();
	Initialize();

	float Dt = 0;
	while (!glfwWindowShouldClose(Window)) {
		double CurTime = glfwGetTime();
		Dt = CurTime - Time;
		Time = CurTime;

		AladinUpdateBegin(Dt);
		Loop(Dt);
		AladinUpdateEnd(Dt);
	}

	ImGui::DestroyContext();
	glfwTerminate();
	return 0;
}

void ImGui_Draw(ImDrawData* Data) {
	ImGuiIO& IO = ImGui::GetIO();
	float FrameWidth = IO.DisplaySize.x * IO.DisplayFramebufferScale.x;
	float FrameHeight = IO.DisplaySize.y * IO.DisplayFramebufferScale.y;
	if (FrameWidth == 0 || FrameHeight == 0)
		return;

	Data->ScaleClipRects(IO.DisplayFramebufferScale);

	alSetView(FrameWidth, FrameHeight, IO.DisplaySize.x, IO.DisplaySize.y);
	alSetScissor(0, 0, -1, -1);
	alClear(0.5f, 0.5f, 0.5f, 1);

	for (int n = 0; n < Data->CmdListsCount; n++) {
		const ImDrawList* CmdList = Data->CmdLists[n];
		const ImDrawVert* Verts = CmdList->VtxBuffer.Data;
		const ImDrawIdx* Idx = CmdList->IdxBuffer.Data;

		alSetVertices(2, sizeof(ImDrawVert), (void*)((const char*)Verts + IM_OFFSETOF(ImDrawVert, pos)));
		alSetTextureCoords(sizeof(ImDrawVert), (void*)((const char*)Verts + IM_OFFSETOF(ImDrawVert, uv)));
		alSetColors(sizeof(ImDrawVert), (void*)((const char*)Verts + IM_OFFSETOF(ImDrawVert, col)));

		for (int i = 0; i < CmdList->CmdBuffer.Size; i++) {
			const ImDrawCmd* DrawCmd = &CmdList->CmdBuffer[i];

			if (DrawCmd->UserCallback)
				DrawCmd->UserCallback(CmdList, DrawCmd);
			else {
				// Draw
				alSetScissor(DrawCmd->ClipRect.x, (FrameHeight - DrawCmd->ClipRect.w), (DrawCmd->ClipRect.z - DrawCmd->ClipRect.x), (DrawCmd->ClipRect.w - DrawCmd->ClipRect.y));
				alUseTexture((int)DrawCmd->TextureId);
				alDrawTriangles(DrawCmd->ElemCount, (void*)Idx);
			}

			Idx += DrawCmd->ElemCount;
		}
	}
}

const char* AladinGetClipboardText(void* W) {
	return glfwGetClipboardString((GLFWwindow*)W);
}

void AladinSetClipboardText(void* W, const char* Text) {
	glfwSetClipboardString((GLFWwindow*)W, Text);
}


void AladinMouseButtonCallback(GLFWwindow*, int Button, int Action, int) {
	if (Action == GLFW_PRESS && Button >= 0 && Button < 3)
		MouseJustPressed[Button] = true;
}

void AladinScrollCallback(GLFWwindow*, double XOffset, double YOffset) {
	ImGuiIO& IO = ImGui::GetIO();
	IO.MouseWheelH += (float)XOffset;
	IO.MouseWheel += (float)YOffset;
}

void AladinKeyCallback(GLFWwindow*, int Key, int, int Action, int) {
	ImGuiIO& IO = ImGui::GetIO();
	if (Action == GLFW_PRESS)
		IO.KeysDown[Key] = true;
	if (Action == GLFW_RELEASE)
		IO.KeysDown[Key] = false;

	IO.KeyCtrl = IO.KeysDown[GLFW_KEY_LEFT_CONTROL] || IO.KeysDown[GLFW_KEY_RIGHT_CONTROL];
	IO.KeyShift = IO.KeysDown[GLFW_KEY_LEFT_SHIFT] || IO.KeysDown[GLFW_KEY_RIGHT_SHIFT];
	IO.KeyAlt = IO.KeysDown[GLFW_KEY_LEFT_ALT] || IO.KeysDown[GLFW_KEY_RIGHT_ALT];
	IO.KeySuper = IO.KeysDown[GLFW_KEY_LEFT_SUPER] || IO.KeysDown[GLFW_KEY_RIGHT_SUPER];
}

void AladinCharCallback(GLFWwindow*, unsigned int C) {
	ImGuiIO& IO = ImGui::GetIO();
	if (C > 0 && C < 0x10000)
		IO.AddInputCharacter((unsigned short)C);
}

void InitGUI() {
	glfwSetMouseButtonCallback(Window, AladinMouseButtonCallback);
	glfwSetScrollCallback(Window, AladinScrollCallback);
	glfwSetKeyCallback(Window, AladinKeyCallback);
	glfwSetCharCallback(Window, AladinCharCallback);

	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
	ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 0);
	ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarRounding, 0);

	ImGuiIO& IO = ImGui::GetIO();
	IO.KeyMap[ImGuiKey_Tab] = GLFW_KEY_TAB;
	IO.KeyMap[ImGuiKey_LeftArrow] = GLFW_KEY_LEFT;
	IO.KeyMap[ImGuiKey_RightArrow] = GLFW_KEY_RIGHT;
	IO.KeyMap[ImGuiKey_UpArrow] = GLFW_KEY_UP;
	IO.KeyMap[ImGuiKey_DownArrow] = GLFW_KEY_DOWN;
	IO.KeyMap[ImGuiKey_PageUp] = GLFW_KEY_PAGE_UP;
	IO.KeyMap[ImGuiKey_PageDown] = GLFW_KEY_PAGE_DOWN;
	IO.KeyMap[ImGuiKey_Home] = GLFW_KEY_HOME;
	IO.KeyMap[ImGuiKey_End] = GLFW_KEY_END;
	IO.KeyMap[ImGuiKey_Insert] = GLFW_KEY_INSERT;
	IO.KeyMap[ImGuiKey_Delete] = GLFW_KEY_DELETE;
	IO.KeyMap[ImGuiKey_Backspace] = GLFW_KEY_BACKSPACE;
	IO.KeyMap[ImGuiKey_Space] = GLFW_KEY_SPACE;
	IO.KeyMap[ImGuiKey_Enter] = GLFW_KEY_ENTER;
	IO.KeyMap[ImGuiKey_Escape] = GLFW_KEY_ESCAPE;
	IO.KeyMap[ImGuiKey_A] = GLFW_KEY_A;
	IO.KeyMap[ImGuiKey_C] = GLFW_KEY_C;
	IO.KeyMap[ImGuiKey_V] = GLFW_KEY_V;
	IO.KeyMap[ImGuiKey_X] = GLFW_KEY_X;
	IO.KeyMap[ImGuiKey_Y] = GLFW_KEY_Y;
	IO.KeyMap[ImGuiKey_Z] = GLFW_KEY_Z;

	IO.SetClipboardTextFn = AladinSetClipboardText;
	IO.GetClipboardTextFn = AladinGetClipboardText;
	IO.ClipboardUserData = Window;

	unsigned char* Tex = NULL;
	int TexW, TexH;
	IO.Fonts->GetTexDataAsRGBA32(&Tex, &TexW, &TexH);
	IO.Fonts->TexID = (ImTextureID)alCreateTexture(TexW, TexH, Tex);
}

void AladinUpdateBegin(float Dt) {
	glfwPollEvents();
	glfwGetWindowSize(Window, &Width, &Height);
	glfwGetFramebufferSize(Window, &FramebufferWidth, &FramebufferHeight);

	ImGuiIO& IO = ImGui::GetIO();
	IO.DisplaySize = ImVec2(Width, Height);
	IO.DisplayFramebufferScale = ImVec2(Width > 0 ? ((float)FramebufferWidth / Width) : 0, Height > 0 ? ((float)FramebufferHeight / Height) : 0);
	IO.DeltaTime = Dt;

	if (glfwGetWindowAttrib(Window, GLFW_FOCUSED)) {
		if (IO.WantMoveMouse)
			glfwSetCursorPos(Window, IO.MousePos.x, IO.MousePos.y);
		else {
			double MouseX, MouseY;
			glfwGetCursorPos(Window, &MouseX, &MouseY);
			IO.MousePos = ImVec2(MouseX, MouseY);
		}
	}
	else
		IO.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);

	for (int i = 0; i < sizeof(MouseJustPressed); i++) {
		IO.MouseDown[i] = MouseJustPressed[i] || glfwGetMouseButton(Window, i) != 0;
		MouseJustPressed[i] = false;
	}

	glfwSetInputMode(Window, GLFW_CURSOR, IO.MouseDrawCursor ? GLFW_CURSOR_HIDDEN : GLFW_CURSOR_NORMAL);
	ImGui::NewFrame();
}

void AladinUpdateEnd(float Dt) {
	ImGui::Render();
	ImGui_Draw(ImGui::GetDrawData());
	glfwSwapBuffers(Window);
}

///////////////////////////////////////////////////////////////// Convenience functions below this line, TODO: move to another file

// TODO: Better implementation; https://github.com/vercas/Beelzebub/blob/master/libs/common/src/cmd_options.cpp#L72
char** AladinSplitString(char* Str, const char* Delim) {
	char** Ret = NULL;
	int Count = 0;

	char* Duplicate = strdup(Str);
	char* P = strtok(Duplicate, Delim);
	while (P != NULL) {
		Count++;
		P = strtok(NULL, Delim);
	}
	free(Duplicate);

	Ret = (char**)calloc(Count + 1, sizeof(char*));
	Count = 0;

	P = strtok(Str, Delim);
	while (P != NULL) {
		Ret[Count++] = P;
		P = strtok(NULL, Delim);
	}

	return Ret;
}

int AladinStringArrayLen(const char** Arr) {
	int Len = 0;

	while (Arr[Len++])
		;

	return Len - 1;
}