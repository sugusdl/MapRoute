#pragma once
#include <vector>
#include <cmath>
#include <algorithm>
#include <atlimage.h>
#include <afxwin.h> 
class CChildView : public CWnd
{
public:
    CChildView();
    virtual ~CChildView();

protected:
    CImage m_bgImage;

    // 그래프 자료구조
    struct Node { CPoint pt; };
    struct Edge { int a, b; double weight; };

    std::vector<Node> g_nodes;    
    std::vector<Edge> g_edges;     
    std::vector<int> g_selected;  
    std::vector<int> m_lastPath;   

    double Dist(CPoint a, CPoint b);
    std::vector<int> RunDijkstra(int s, int e);

protected:
    afx_msg void OnPaint();
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);

    DECLARE_MESSAGE_MAP()
};
