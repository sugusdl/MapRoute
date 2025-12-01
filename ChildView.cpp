#include "pch.h"
#include "framework.h"
#include "MapRoute.h"
#include "ChildView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(CChildView, CWnd)
    ON_WM_PAINT()
    ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()

CChildView::CChildView()
{
    HRESULT hr = m_bgImage.Load(_T("map.png"));  
}

CChildView::~CChildView()
{
    if (!m_bgImage.IsNull())
        m_bgImage.Destroy();
}

double CChildView::Dist(CPoint a, CPoint b)
{
    double dx = a.x - b.x;
    double dy = a.y - b.y;
    return sqrt(dx * dx + dy * dy);
}

std::vector<int> CChildView::RunDijkstra(int start, int end)
{
    int N = (int)g_nodes.size();
    if (N == 0) return {};

    const double INF = 1e18;
    std::vector<double> dist(N, INF);
    std::vector<int> prev(N, -1);
    std::vector<bool> used(N, false);

    dist[start] = 0;

    for (int i = 0; i < N; i++)
    {
        int u = -1;
        for (int j = 0; j < N; j++)
        {
            if (!used[j] && (u == -1 || dist[j] < dist[u]))
                u = j;
        }
        if (u == -1 || dist[u] == INF) break;

        used[u] = true;

        for (auto& e : g_edges)
        {
            if (e.a == u)
            {
                if (dist[e.b] > dist[u] + e.weight)
                {
                    dist[e.b] = dist[u] + e.weight;
                    prev[e.b] = u;
                }
            }
            if (e.b == u)
            {
                if (dist[e.a] > dist[u] + e.weight)
                {
                    dist[e.a] = dist[u] + e.weight;
                    prev[e.a] = u;
                }
            }
        }
    }

    std::vector<int> path;
    for (int cur = end; cur != -1; cur = prev[cur])
        path.push_back(cur);

    std::reverse(path.begin(), path.end());
    if (path.size() < 2 || path[0] != start)
        return {};

    return path;
}

void CChildView::OnPaint()
{
    CPaintDC dc(this);

    if (!m_bgImage.IsNull())
    {
        CRect rect;
        GetClientRect(&rect);
        m_bgImage.Draw(dc, rect);
    }

    // 점 그리기
    for (auto& n : g_nodes)
    {
        CBrush brush(RGB(0, 0, 0));
        CBrush* oldBrush = dc.SelectObject(&brush);
        int radius = 8;
        dc.Ellipse(n.pt.x - radius, n.pt.y - radius,
            n.pt.x + radius, n.pt.y + radius);
        dc.SelectObject(oldBrush);
    }

    // Alt로 연결한 선
    CPen penAlt(PS_SOLID, 3, RGB(0, 0, 255));
    CPen* oldPen = dc.SelectObject(&penAlt);
    for (auto& e : g_edges)
    {
        CPoint p1 = g_nodes[e.a].pt;
        CPoint p2 = g_nodes[e.b].pt;
        dc.MoveTo(p1);
        dc.LineTo(p2);
    }
    dc.SelectObject(oldPen);

    // Ctrl+Shift 최단 경로 
    if (!m_lastPath.empty())
    {
        CPen penPath(PS_SOLID, 4, RGB(255, 0, 0));
        oldPen = dc.SelectObject(&penPath);

        for (int i = 0; i + 1 < m_lastPath.size(); i++)
        {
            int a = m_lastPath[i];
            int b = m_lastPath[i + 1];
            CPoint p1 = g_nodes[a].pt;
            CPoint p2 = g_nodes[b].pt;
            dc.MoveTo(p1);
            dc.LineTo(p2);

            CString str;
            double w = Dist(p1, p2);
            str.Format(_T("%.1f"), w);
            CPoint mid((p1.x + p2.x) / 2, (p1.y + p2.y) / 2);
            dc.TextOut(mid.x, mid.y, str);
        }
        dc.SelectObject(oldPen);
    }
}

void CChildView::OnLButtonDown(UINT nFlags, CPoint point)
{
    BOOL ctrl = (GetKeyState(VK_CONTROL) & 0x8000);
    BOOL shift = (GetKeyState(VK_SHIFT) & 0x8000);
    BOOL alt = (GetKeyState(VK_MENU) & 0x8000);

    //  Ctrl + Click → 점 추가
    if (ctrl && !shift && !alt)
    {
        g_nodes.push_back({ point });
        Invalidate();
        return;
    }

    //  Alt + Click → 두 점 선택 → 엣지 연결
    if (alt && !ctrl && !shift)
    {
        int sel = -1;
        for (int i = 0; i < g_nodes.size(); i++)
        {
            if (Dist(g_nodes[i].pt, point) < 10)
            {
                sel = i;
                break;
            }
        }
        if (sel != -1)
        {
            g_selected.push_back(sel);
            if (g_selected.size() == 2)
            {
                int a = g_selected[0];
                int b = g_selected[1];
                g_edges.push_back({ a, b, Dist(g_nodes[a].pt, g_nodes[b].pt) });
                g_selected.clear();
            }
        }
        Invalidate();
        return;
    }

    //  Ctrl + Shift + Click → 최단경로
    if (ctrl && shift && !alt)
    {
        int sel = -1;
        for (int i = 0; i < g_nodes.size(); i++)
        {
            if (Dist(g_nodes[i].pt, point) < 10)
            {
                sel = i;
                break;
            }
        }
        if (sel != -1)
        {
            g_selected.push_back(sel);
            if (g_selected.size() == 2)
            {
                int s = g_selected[0];
                int e = g_selected[1];
                m_lastPath = RunDijkstra(s, e);
                g_selected.clear();
            }
        }
        Invalidate();
        return;
    }

    CWnd::OnLButtonDown(nFlags, point);
}
