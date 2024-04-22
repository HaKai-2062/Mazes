#pragma once

#include <map>
#include <stack>

class Maze
{
public:

    Maze() = delete;
    Maze(Maze&) = delete;
    Maze(Maze&&) = delete;

    Maze(uint16_t width, uint16_t height, uint16_t cellWidth = 10, uint16_t wallThickness = 2)
        : m_MazeWidth(width), m_MazeHeight(height), m_HalfCellHeight(cellWidth), m_WallThickness(wallThickness)
    {
        std::cout << "Maze Created [" << width << "x" << height << "]" << std::endl;
        m_TotalCellHeight = 2 * m_HalfCellHeight + m_WallThickness;
        m_CellsAcrossWidth = static_cast<uint16_t>(floor(width / m_TotalCellHeight));
        m_CellsAcrossHeight = static_cast<uint16_t>(floor(height / m_TotalCellHeight));
        m_MazeArea = m_CellsAcrossHeight * m_CellsAcrossWidth;

        m_CellOrigin.resize(m_MazeArea);
        // Specifying color using uniform will be way more efficient
        // Corners use 1 vertex, Colors use 2 vertices
        // 12 Vertices make a cell: 4 corners + 4 colors
        m_Vertices.resize(static_cast<size_t>(12 * m_MazeArea));
        m_Indices.resize(static_cast<size_t>(6 * m_MazeArea));

        m_LineVertices.resize(static_cast<size_t>(12 * m_MazeArea));
        m_LineIndices.resize(static_cast<size_t>(6 * m_MazeArea));
    }

	~Maze()
	{
		std::cout << "Maze Destroyed" << std::endl;
	}

	uint32_t DrawMaze(std::stack<uint32_t>* stack = nullptr, std::pair<uint32_t, uint32_t>* route = nullptr)
	{
        uint32_t currentCell = 0;

        auto colorOfVertex = [&]()
        {
            // RG, BA color of each block
            if (stack && !stack->empty() && currentCell == stack->top())
            {
                m_Vertices.push_back(std::make_pair(m_ColorStackTop[0], m_ColorStackTop[1]));
                m_Vertices.push_back(std::make_pair(m_ColorStackTop[2], m_ColorStackTop[3]));
            }
            else if (route && currentCell == route->first)
            {
                m_Vertices.push_back(std::make_pair(m_ColorStart[0], m_ColorStart[1]));
                m_Vertices.push_back(std::make_pair(m_ColorStart[2], m_ColorStart[3]));
            }
            else if (route && currentCell == route->second)
            {
                m_Vertices.push_back(std::make_pair(m_ColorEnd[0], m_ColorEnd[1]));
                m_Vertices.push_back(std::make_pair(m_ColorEnd[2], m_ColorEnd[3]));
            }
            else if (m_VisitedCellInfo.find(currentCell) != m_VisitedCellInfo.end() && (m_VisitedCellInfo[currentCell] & Maze::CELL_SEARCHED))
            {
                m_Vertices.push_back(std::make_pair(m_ColorSearched[0], m_ColorSearched[1]));
                m_Vertices.push_back(std::make_pair(m_ColorSearched[2], m_ColorSearched[3]));
            }
            else if (m_VisitedCellInfo.find(currentCell) != m_VisitedCellInfo.end() && (m_VisitedCellInfo[currentCell] & Maze::CELL_VISITED))
            {
                m_Vertices.push_back(std::make_pair(m_ColorMaze[0], m_ColorMaze[1]));
                m_Vertices.push_back(std::make_pair(m_ColorMaze[2], m_ColorMaze[3]));
            }
            else
            {
                m_Vertices.push_back(std::make_pair(m_ColorBackground[0], m_ColorBackground[1]));
                m_Vertices.push_back(std::make_pair(m_ColorBackground[2], m_ColorStackTop[3]));
            }
        };

        // We are multiplying by 2 because the coordinate go from -1 to 1 instead of 0 to 1
        // Easier to think as dividing by 2 in denominator
        float normalizedHalfCellWidth = static_cast<float>(2 * m_HalfCellHeight) / (m_MazeWidth);
        float normalizedHalfCellHeight = static_cast<float>(2 * m_HalfCellHeight) / (m_MazeHeight);
        float normalizedWallThickness = static_cast<float>(2 * m_WallThickness) / (m_MazeHeight);

        float normalizedTotalCellWidth = static_cast<float>(2 * m_TotalCellHeight) / (m_MazeWidth);
        float normalizedTotalCellHeight = static_cast<float>(2 * m_TotalCellHeight) / (m_MazeHeight);

        float normalizedLineThickness = static_cast<float>(2 * m_LineThickness) / (m_MazeHeight);

        // This goes from bottomRight (-1, -1) to topLeft (1, 1)
        // WIDTH
        for (float i = -1.0f; i < 1.0f; i += normalizedTotalCellWidth)
        {
            // To not draw out of bound cells
            if (i + 2 * normalizedHalfCellWidth > 1)
            {
                continue;
            }
            // HEIGHT
            for (float j = -1.0f; j < 1.0f; j += normalizedTotalCellHeight)
            {
                if (j + 2 * normalizedHalfCellHeight > 1)
                {
                    continue;
                }

                bool isWallEast = true;
                if (m_VisitedCellInfo.find(currentCell) != m_VisitedCellInfo.end() && (m_VisitedCellInfo[currentCell] & Maze::CELL_EAST))
                    isWallEast = false;

                bool isWallSouth = true;
                if (m_VisitedCellInfo.find(currentCell) != m_VisitedCellInfo.end() && (m_VisitedCellInfo[currentCell] & Maze::CELL_SOUTH))
                    isWallSouth = false;

                m_Indices.push_back((4 * currentCell) + 0);
                m_Indices.push_back((4 * currentCell) + 1);
                m_Indices.push_back((4 * currentCell) + 3);
                m_Indices.push_back((4 * currentCell) + 1);
                m_Indices.push_back((4 * currentCell) + 2);
                m_Indices.push_back((4 * currentCell) + 3);

                m_CellOrigin.push_back(std::make_pair<float, float>(i + normalizedHalfCellWidth, j + normalizedHalfCellHeight));

                // Draw the cell
                // right bottom
                m_Vertices.push_back(std::make_pair<float, float>(m_CellOrigin[currentCell].first + normalizedHalfCellWidth + (isWallEast ? 0 : normalizedWallThickness), m_CellOrigin[currentCell].second + normalizedHalfCellHeight));
                colorOfVertex();
                // right top
                m_Vertices.push_back(std::make_pair<float, float>(m_CellOrigin[currentCell].first + normalizedHalfCellWidth + (isWallEast ? 0 : normalizedWallThickness), m_CellOrigin[currentCell].second - normalizedHalfCellHeight - (isWallSouth ? 0 : normalizedWallThickness)));
                colorOfVertex();
                // left bottom
                m_Vertices.push_back(std::make_pair<float, float>(m_CellOrigin[currentCell].first - normalizedHalfCellWidth, m_CellOrigin[currentCell].second - normalizedHalfCellHeight - (isWallSouth ? 0 : normalizedWallThickness)));
                colorOfVertex();
                // left top
                m_Vertices.push_back(std::make_pair<float, float>(m_CellOrigin[currentCell].first - normalizedHalfCellWidth, m_CellOrigin[currentCell].second + normalizedHalfCellHeight));
                colorOfVertex();
                
                currentCell++;
            }
        }

        return currentCell;
	}

    bool MazeCompleted() const
    {
        return m_VisitedCellCount == m_CellsAcrossWidth * m_CellsAcrossHeight;
    }

public:
    // This is made public to let our algorithm access it
    // TDL: Do I really need getters/setters?
    
    // 1st int is cell number,
    // 2nd int represents the neighbours and if it has been visited or not
    std::map<uint32_t, uint16_t> m_VisitedCellInfo;

    const enum
    {
        CELL_NORTH = 0x01,
        CELL_EAST = 0x02,
        CELL_SOUTH = 0x04,
        CELL_WEST = 0x08,
        CELL_VISITED = 0x10,
        CELL_SEARCHED = 0x20
    };

    float m_ColorMaze[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    float m_ColorStart[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
    float m_ColorEnd[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
    float m_ColorStackTop[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
    float m_ColorSearched[4] = { 0.8f, 0.8f, 0.8f, 1.0f };
    float m_ColorBackground[4] = { 1.0f, 0.5f, 0.2f, 1.0f };
    
    // These values are in pixels
    uint16_t m_HalfCellHeight = 10;
    uint16_t m_WallThickness = 2;
    uint16_t m_TotalCellHeight = 0;
    uint16_t m_LineThickness = 4;

	uint16_t m_MazeWidth = 0, m_MazeHeight = 0;
	uint32_t m_MazeArea, m_CellsAcrossWidth = 0, m_CellsAcrossHeight = 0, m_VisitedCellCount = 0;

    std::vector<std::pair<float, float>> m_CellOrigin;

    std::vector<std::pair<float, float>> m_Vertices;
    std::vector<uint32_t> m_Indices;

    std::vector<std::pair<float, float>> m_LineVertices;
    std::vector<uint32_t> m_LineIndices;
};