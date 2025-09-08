#pragma once

#include "globalincs/pstypes.h"

#include "mission/dialogs/CampaignEditorDialogModel.h"

#include <QGraphicsObject>
#include <QGraphicsPathItem>
#include <QGraphicsView>
#include <QPointer>
#include <vector>

// Forward declaration to avoid coupling for now
namespace fso::fred::dialogs {
class CampaignEditorDialogModel;
enum class CampaignSpecialMode;
} // namespace fso::fred::dialogs

class QGraphicsScene;

// Forward-declare internal items so we can use pointers before the full namespace block
namespace detail {
class MissionNodeItem;
class EdgeItem;
class EndSinkItem;
} // namespace detail

/**
 * Visual style knobs for the graph. Kept public so both the graph and items use the same values.
 */
struct CampaignGraphStyle {
	// Background & grid
	QColor bgColor{245, 245, 245};
	QColor gridMinor{230, 230, 230};
	QColor gridMajor{210, 210, 210};
	qreal minorStep{32.0};
	qreal majorEvery{4.0};

	// Node card
	QColor nodeFill{230, 230, 230}; // Qt grey
	QColor nodeBorder{20, 20, 20};
	qreal nodeRadius{8.0};
	QSizeF nodeSize{240.0, 120.0}; // base size; can grow later
	qreal padding{8.0};

	// Stripe (user color)
	qreal stripeHeight{10.0};

	// Ports (nubs)
	QColor inboundGreen{46, 204, 113}; // top nub (inbound)
	QColor mainBlue{52, 152, 219};     // main out nub
	QColor loopOrange{243, 156, 18};   // special out nub when LOOP
	QColor forkPurple{155, 89, 182};   // special out nub when FORK
	qreal nubRadius{6.0};
	qreal nubSpacingBottom{6.0}; // retained for future
	qreal nubOffsetX{42.0};

	// Edge routing
	qreal fanoutStart{12.0}; // vertical run from nub before spreading
	qreal fanoutStep{10.0};  // horizontal separation between sibling edges
	qreal edgeWidth{3.0};
	qreal arrowSize{8.0}; // arrowhead size
	qreal arrowTargetInset{10.0};
	qreal arrowInterval{100.0}; // place an arrow every N px along straight segments

	// Self-loop display
	bool showSelfLoops{false};
	qreal selfLoopMargin{32.0}; // how far outside the node to wrap
	qreal selfLoopSpread{10.0}; // per-sibling additional spacing

	// Badge
	qreal badgePad{6.0};
	QSizeF badgeSize{34.0, 18.0}; // pill
	QColor badgeDisabled{180, 180, 180};

	// END sink visuals/placement
	QSizeF endSinkSize{120.0, 36.0};
	QColor endSinkFill{255, 235, 235};
	QColor endSinkBorder{200, 60, 60};
	QColor endSinkText{120, 20, 20};
	qreal endSinkRadius{14.0};
	qreal endSinkMargin{160.0}; // distance below the lowest node row

	qreal contentMarginX{400.0}; // extra scene space left/right of content on rebuild
	qreal contentMarginY{400.0}; // extra scene space above/below content on rebuild
};

/**
 * @brief Canvas for the campaign mission graph.
 */
class CampaignMissionGraph final : public QGraphicsView {
	Q_OBJECT
  public:
	explicit CampaignMissionGraph(QWidget* parent = nullptr);

	// Hook up the working campaign data (read-only from the view)
	void setModel(fso::fred::dialogs::CampaignEditorDialogModel* model);

	// Rebuild/redraw the entire scene from the model
	void rebuildAll();

	void
	setSelectedMission(int missionIndex, bool makeVisible = true, bool centerOnItem = false, bool emitSignal = false);
	void clearSelectedMission();

	// View helpers
	void zoomToFitAll(qreal margin = 40.0);
	void setGridVisible(bool on);
	bool isGridVisible() const
	{
		return m_gridVisible;
	}
	void setZoomEnabled(bool on)
	{
		m_zoomEnabled = on;
	}

  signals:
	// Emitted when a mission node is clicked/selected
	void missionSelected(int missionIndex);
	// Emitted when the LOOP/FORK badge is clicked (only when no special branches exist)
	void specialModeToggleRequested(int missionIndex);
	// (Edges are now non-interactive, keeping this signal around is harmless if you already wired it)
	void branchSelected(int missionIndex, int branchId);

  protected:
	// Pan/zoom
	void wheelEvent(QWheelEvent* e) override;
	// Grid background
	void drawBackground(QPainter* painter, const QRectF& rect) override;

  private:
	void initScene();
	void updateSceneRectToContent(bool scrollToTopLeft);
	void drawGrid(QPainter* p, const QRectF& rect);
	void buildMissionNodes();
	void buildMissionEdges();
	void ensureEndSink();

	QGraphicsScene* m_scene{nullptr}; // owned by view (parented)
	QPointer<fso::fred::dialogs::CampaignEditorDialogModel> m_model;

	// Centralized visuals
	CampaignGraphStyle m_style;

	// Items we create (aligned to model order)
	SCP_vector<detail::MissionNodeItem*> m_nodeItems;
	SCP_vector<detail::EdgeItem*> m_edgeItems;
	QPointer<detail::EndSinkItem> m_endSink{nullptr};

	bool m_gridVisible{true};
	bool m_zoomEnabled{true};
	qreal m_currentScale{1.0};
	const qreal kMinScale{0.2};
	const qreal kMaxScale{3.0};
};

// ---------- Internal items (Q_OBJECT in header so AUTOMOC runs) ----------

namespace detail {

using fso::fred::dialogs::CampaignSpecialMode;

/**
 * One mission node item.
 */
class MissionNodeItem final : public QGraphicsObject {
	Q_OBJECT
  public:
	MissionNodeItem(int missionIndex,
		const QString& fileLabel,
		const QString& nameLabel,
		int graphColorRgb, // -1 = none else 0xRRGGBB
		CampaignSpecialMode mode,
		int mainBranchCount,
		int specialBranchCount,
		const CampaignGraphStyle& style,
		QGraphicsItem* parent = nullptr);

	QRectF boundingRect() const override;
	void paint(QPainter* p, const QStyleOptionGraphicsItem* opt, QWidget* w) override;

	// Nub anchor points (scene coordinates)
	QPointF inboundNubScenePos() const;
	QPointF mainNubScenePos() const;
	QPointF specialNubScenePos() const;

  signals:
	void missionSelected(int missionIndex);
	void specialModeToggleRequested(int missionIndex);

  protected:
	void mousePressEvent(QGraphicsSceneMouseEvent* e) override;
	void mouseReleaseEvent(QGraphicsSceneMouseEvent* e) override;

  private:
	void updateGeometry();

  private:
	int m_idx{-1};
	QString m_file;
	QString m_name;
	int m_graphColor{-1};
	CampaignSpecialMode m_mode{CampaignSpecialMode::Loop};
	int m_mainCount{0};
	int m_specCount{0};

	const CampaignGraphStyle& m_style;

	QRectF m_rect;
	QRectF m_badgeRect;
	qreal m_titleH{18.0};
	qreal m_nameH{24.0};
};

/**
 * Edge between missions (with arrowhead). Drawn with Manhattan routing and fan-out.
 * Non-interactive: edges ignore mouse events and are not selectable.
 */
class EdgeItem final : public QObject, public QGraphicsPathItem {
	Q_OBJECT
  public:
	EdgeItem(int missionIndex,
		int branchId,
		bool isSpecial,
		CampaignSpecialMode mode,
		const CampaignGraphStyle& style,
		QGraphicsItem* parent = nullptr);

	// Standard (non-self) edge
	void setEndpoints(const QPointF& src, const QPointF& dst, int siblingIndex, int siblingCount);

	// Self-loop variant (wrap outside node rect on the appropriate side)
	void setSelfLoop(const QRectF& nodeRectScene, bool sourceIsRightSide, int siblingIndex, int siblingCount);

	// (kept for completeness; not used now)
	void setSelectedVisual(bool sel);

  protected:
	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

  private:
	QPainterPath buildPath(const QPointF& src, const QPointF& dst, int siblingIndex, int siblingCount);
	QPainterPath
	buildSelfLoopPath(const QRectF& nodeRectScene, bool sourceIsRightSide, int siblingIndex, int siblingCount);

  private:
	int m_missionIndex{-1};
	int m_branchId{-1};
	bool m_isSpecial{false};
	CampaignSpecialMode m_mode{CampaignSpecialMode::Loop};
	const CampaignGraphStyle& m_style;

	QColor m_color;
	Qt::PenStyle m_dash{Qt::SolidLine};

	// Cache for arrow drawing
	QPointF m_lastSegmentP1; // second-to-last point
	QPointF m_lastSegmentP2; // last point (path end)

	SCP_vector<QPointF> m_points; // cached polyline points for arrow placement
};

/**
 * End of campaign pill to visualize the "END" target for branches with no next mission.
 */
class EndSinkItem final : public QGraphicsObject {
	Q_OBJECT
  public:
	explicit EndSinkItem(const CampaignGraphStyle& style, QGraphicsItem* parent = nullptr)
		: QGraphicsObject(parent), m_style(style)
	{
	}

	QRectF boundingRect() const override
	{
		// Expand upward by nub radius so the top nub isn't clipped
		QRectF pill(QPointF(0, 0), m_style.endSinkSize);
		return pill.adjusted(-1.0, -m_style.nubRadius, +1.0, +1.0);
	}

	void paint(QPainter* p, const QStyleOptionGraphicsItem*, QWidget*) override
	{
		p->setRenderHint(QPainter::Antialiasing, true);

		// Geometry: draw relative to the pill rect (not the expanded boundingRect)
		const QRectF pill(QPointF(0, 0), m_style.endSinkSize);

		// 1) Inbound nub (green), centered on the pill's top edge so only half shows
		QPen nubPen(m_style.endSinkBorder, 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
		p->setPen(nubPen);
		p->setBrush(m_style.inboundGreen);
		const QPointF nubCenter(pill.center().x(), pill.top());
		p->drawEllipse(nubCenter, m_style.nubRadius, m_style.nubRadius);

		// 2) Pill on top
		QPen border(m_style.endSinkBorder, 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
		p->setPen(border);
		p->setBrush(m_style.endSinkFill);
		p->drawRoundedRect(pill, m_style.endSinkRadius, m_style.endSinkRadius);

		// 3) Label
		p->setPen(m_style.endSinkText);
		QFont f = p->font();
		f.setBold(true);
		p->setFont(f);
		p->drawText(pill, Qt::AlignCenter, QStringLiteral("END"));
	}

	// Anchor where inbound edges terminate (top-center of pill)
	QPointF inboundAnchorScenePos() const
	{
		// Anchor on the pill's top-center (independent of expanded boundingRect)
		const QRectF pill(QPointF(0, 0), m_style.endSinkSize);
		return mapToScene(QPointF(pill.center().x(), pill.top()));
	}


  private:
	const CampaignGraphStyle& m_style;
};

} // namespace detail
