#include "VisualNovel.h"

namespace Hazel {

    void VisualNovel::LoadScript(const std::string& path)
    {
        std::ifstream file(path);
        std::string line;

        while (std::getline(file, line))
        {
            if (line.rfind("sheet", 0) == 0)
            {
                std::stringstream ss(line);

                std::string tag;
                std::string texture;
                float w, h;

                ss >> tag >> texture >> w >> h;

                m_SpriteSheet = Texture2D::Create("assets/textures/" + texture);

                m_CellSize = { w, h };
            }
            else if (line.rfind("char", 0) == 0)
            {
                std::stringstream ss(line);

                std::string tag;
                std::string name;

                int x;
                int y;

                ss >> tag >> name >> x >> y;

                Characters[name] =
                    SubTexture2D::CreateFromCoords(
                        m_SpriteSheet,
                        { x, y },
                        m_CellSize
                    );
            }
            else
            {
                size_t pos = line.find(":");

                if (pos != std::string::npos)
                {
                    DialogueLine d;

                    d.Speaker = line.substr(0, pos);
                    d.Text = line.substr(pos + 1);

                    m_Lines.push_back(d);
                }
            }
        }
    }

    void VisualNovel::Next()
    {
        if (m_CurrentIndex + 1 < m_Lines.size())
            m_CurrentIndex++;
    }

}