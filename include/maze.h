#pragma once

#include <vector>
#include <map>
#include <stack>
#include <stdint.h>

class Maze
{
public:

    Maze() = delete;
    Maze(Maze&) = delete;
    Maze(Maze&&) = delete;

    Maze(uint16_t width, uint16_t height, uint16_t cellWidth = 10, uint16_t wallThickness = 2);
    ~Maze();

    uint32_t DrawMaze(std::vector<uint32_t>* stack = nullptr, std::pair<uint32_t, uint32_t>* route = nullptr);
    bool MazeCompleted() const;
    // Each cell does std::abs(cellWeight+neighbourWeight) to get corresponding weight
    void SetCellWeights();

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
    float m_ColorSearchTop[4] = { 0.8f, 0.8f, 0.8f, 1.0f };
    float m_ColorSearched[4] = { 0.8f, 0.8f, 0.8f, 1.0f };
    float m_ColorBackground[4] = { 1.0f, 0.5f, 0.2f, 1.0f };
    
    // These values are in pixels
    uint16_t m_HalfCellHeight = 10;
    uint16_t m_WallThickness = 2;
    uint16_t m_TotalCellHeight = 0;
    uint16_t m_LineThickness = 1;

	uint16_t m_MazeWidth = 0, m_MazeHeight = 0;
	uint32_t m_MazeArea, m_CellsAcrossWidth = 0, m_CellsAcrossHeight = 0, m_VisitedCellCount = 0;

    std::vector<std::pair<float, float>> m_CellOrigin;

    std::vector<std::pair<float, float>> m_Vertices;
    std::vector<uint32_t> m_Indices;

    std::vector<std::pair<float, float>> m_LineVertices;
    std::vector<uint32_t> m_LineIndices;

    std::vector<uint32_t> m_CellWeights;
    int m_RandUpperLimit = 30;
};