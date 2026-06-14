#pragma once

#include "Hazel.h"

namespace Hazel {

	struct DialogueLine
	{
		std::string Speaker;
		std::string Text;
	};

	class VisualNovel
	{
	public:
		void LoadScript(const std::string& path);

		void Next();

		const DialogueLine& GetCurrentLine() const { return m_Lines[m_CurrentIndex]; }
		const uint32_t GetLineCount() const { return m_Lines.size(); }

		std::unordered_map<std::string, Ref<SubTexture2D>> Characters;

	private:
		Ref<Texture2D> m_SpriteSheet;
		glm::vec2 m_CellSize;
		std::vector<DialogueLine> m_Lines;
		uint32_t m_CurrentIndex = 0;
	};

}