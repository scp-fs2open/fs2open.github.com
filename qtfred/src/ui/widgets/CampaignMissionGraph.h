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

namespace detail {
class MissionNodeItem;
class EdgeItem;
class EndSinkItem;
} // namespace detail

/**
 * Visual style knobs for the graph. Kept public so both the graph and items use the same values.
 */
struct CampaignGraphStyle {
	// Global
	bool forksEnabled{false}; // if true, FORK special branches are allowed. Disabled for now since the feature is incomplete.

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
	QSizeF nodeSize{240.0, 120.0}; // base size
	qreal padding{8.0};
	qreal layoutStepX{240.0}; // default horizontal spacing for fallback layout from position/level
	qreal layoutStepY{200.0}; // default vertical   spacing for fallback layout from position/level

	// User color stripe
	qreal stripeHeight{10.0};

	// Ports
	QColor inboundPortColor{46, 204, 113}; // top port
	QColor standardPortColor{52, 152, 219};     // main out port
	QColor loopPortColor{243, 156, 18};   // special out port when LOOP
	QColor forkPortColor{155, 89, 182};   // special out port when FORK
	qreal portRadius{6.0};
	qreal portOffsetX{42.0};
	bool inboundApproachEnabled{true}; // add a small jog near the inbound port
	qreal inboundApproachJog{30.0};    // horizontal jog distance before the final vertical
	qreal inboundApproachRise{30.0};   // vertical distance above inbound port
	bool outboundApproachEnabled{true}; // add a small jog right after the source port
	bool outboundDirectionTowardTarget{true}; // if true, outbound jog heads toward dst.x()
	qreal outboundJogMain{30.0};              // X jog right after source for MAIN
	qreal outboundJogSpecial{30.0};           // X jog right after source for SPECIAL
	qreal outboundDropMain{30.0};             // Y drop right after source for MAIN
	qreal outboundDropSpecial{40.0};          // Y drop right after source for SPECIAL
	qreal portHitExtra{30.0};                 // extra pixels added to port radius for hit testing

	// Edge routing
	qreal fanoutStart{12.0}; // vertical run from port before spreading
	qreal fanoutStep{10.0};  // horizontal separation between sibling edges
	qreal edgeWidth{2.0};
	qreal arrowSize{8.0}; // arrowhead size
	qreal arrowTargetInset{10.0};
	qreal arrowInterval{100.0}; // place an arrow every N px along straight segments

	// Self-loop display
	bool showSelfLoops{false};  // if true, self loops are drawn with edges. Disabled because it clutters the view.
	qreal selfLoopMargin{32.0}; // how far outside the node to wrap
	qreal selfLoopSpread{10.0}; // per sibling additional spacing

	// Badge
	qreal badgePad{6.0}; // Used to toggle between LOOP or FORK
	QSizeF badgeSize{34.0, 18.0}; // pill size
	QColor badgeDisabled{180, 180, 180};

	// END sink visuals/placement
	QSizeF endSinkSize{120.0, 36.0};
	QColor endSinkFill{255, 235, 235};
	QColor endSinkBorder{200, 60, 60};
	QColor endSinkText{120, 20, 20};
	qreal endSinkRadius{14.0};
	qreal endSinkMargin{160.0}; // distance below the lowest node row

	// Focus mode / path de-emphasis
	bool focusModeEnabled{true};       // master toggle
	qreal fadedEdgeOpacity{0.25};      // opacity for de-emphasized edges
	qreal highlightedEdgeOpacity{1.0}; // opacity for emphasized edges
	bool focusCountOutgoing{true};     // edges with source == selected are emphasized
	bool focusCountIncoming{true};     // edges with target == selected are emphasized

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

	// Hook up the working campaign data
	void setModel(fso::fred::dialogs::CampaignEditorDialogModel* model);

	// Rebuild/redraw the entire scene from the model
	void rebuildAll(bool refocus = false);

	void
	setSelectedMission(int missionIndex, bool makeVisible = false, bool centerOnItem = false);
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
	bool forksEnabled() const
	{
		return m_style.forksEnabled;
	}

  signals:
	// Emitted when a mission node is clicked/selected
	void missionSelected(int missionIndex);
	// Emitted when the LOOP/FORK badge is clicked and no special branch exists
	void specialModeToggleRequested(int missionIndex);
	// Edges are now noninteractive, keeping this signal for future use
	void branchSelected(int missionIndex, int branchId);
	// Emitted when a request is made to create a new mission node
	void addMissionHereRequested(QPointF sceneTopLeft);
	// Emitted when a request is made to delete a mission node
	void deleteMissionRequested(int missionIndex);
	// Emitted when a request is made to make a mission repeat to self
	void addRepeatBranchRequested(int missionIndex);
	// Emitted when an outbound connection drag is started from a node port and ends in empty space
	void createMissionAtAndConnectRequested(QPointF sceneTopLeft, int fromIndex, bool isSpecial);
	// Emitted when a request is made to make the current mission the first campaign mission
	void setFirstMissionRequested(int missionIndex);

  protected:
	// Pan/zoom
	void wheelEvent(QWheelEvent* e) override;
	// Grid background
	void drawBackground(QPainter* painter, const QRectF& rect) override;
	// Mouse events
	void mousePressEvent(QMouseEvent* e) override;
	void mouseMoveEvent(QMouseEvent* e) override;
	void mouseReleaseEvent(QMouseEvent* e) override;
	// Context menu
	void contextMenuEvent(QContextMenuEvent* e) override;

  private slots:
	void onNodeMoved(int missionIndex, QPointF sceneTopLeft);
	void onSceneSelectionChanged();

  private: // NOLINT(readability-redundant-access-specifiers)
	struct DragState {
		bool active{false};
		bool isSpecial{false};
		int fromIndex{-1};
		QPointF srcPt;
		QPointer<detail::EdgeItem> preview;
	};
	DragState m_drag;

	void initScene();
	void updateSceneRectToContent(bool scrollToTopLeft);
	void drawGrid(QPainter* p, const QRectF& rect) const;
	void buildMissionNodes();
	void buildMissionEdges();
	void ensureEndSink();
	void ensureSceneHooks();
	void rebuildEdgesOnly();
	bool hasRepeatBranch(int missionIndex) const;

	detail::MissionNodeItem* nodeAtScenePos(const QPointF& scenePt) const;
	bool tryFinishConnectionAt(const QPointF& scenePt);
	void cancelDrag();

	void applyFocusEmphasis(); // recompute fade/highlight on all edges

	QGraphicsScene* m_scene{nullptr};
	QPointer<fso::fred::dialogs::CampaignEditorDialogModel> m_model;

	// Centralized visuals
	CampaignGraphStyle m_style;

	SCP_vector<detail::MissionNodeItem*> m_nodeItems;
	SCP_vector<detail::EdgeItem*> m_edgeItems;
	QPointer<detail::EndSinkItem> m_endSink{nullptr};

	bool m_gridVisible{true};
	bool m_zoomEnabled{true};
	qreal m_currentScale{1.0};
	const qreal kMinScale{0.2};
	const qreal kMaxScale{3.0};

	bool m_updatingSceneRect{false};
	bool m_spawnPending{false};

	int m_selectedMissionIndex{-1};

	bool m_internallySelecting{false}; // guard to avoid recursion when we set selection programmatically
	bool m_sceneHooksInstalled{false}; // ensure we only connect once
};

namespace detail {

using fso::fred::dialogs::CampaignSpecialMode;

/**
 * One mission node item.
 */
class MissionNodeItem final : public QGraphicsObject {
	Q_OBJECT
  public:
	MissionNodeItem(int missionIndex,
		QString fileLabel,
		QString nameLabel,
		int graphColorRgb, // -1 = none else 0xRRGGBB
		CampaignSpecialMode mode,
		int mainBranchCount,
		int specialBranchCount,
		CampaignGraphStyle style,
		QGraphicsItem* parent = nullptr);

	QRectF boundingRect() const override;
	void paint(QPainter* p, const QStyleOptionGraphicsItem* opt, QWidget* w) override;

	int missionIndex() const
	{
		return m_idx;
	}


	enum class Port {
		None,
		Inbound,
		Main,
		Special
	};

	Port hitTestPortScene(const QPointF& scenePos) const;

	// Port anchor points
	QPointF inboundPortScenePos() const;
	QPointF mainPortScenePos() const;
	QPointF specialPortScenePos() const;

  signals:
	void missionSelected(int missionIndex);
	void specialModeToggleRequested(int missionIndex);
	void nodeMoved(int missionIndex, QPointF sceneTopLeft);

  protected:
	void mousePressEvent(QGraphicsSceneMouseEvent* e) override;
	void mouseReleaseEvent(QGraphicsSceneMouseEvent* e) override;
	QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

  private:
	void updateGeometry();

	int m_idx{-1};
	QString m_file;
	QString m_name;
	int m_graphColor{-1}; // TODO future ability for users to arbitrarily color mission nodes. Will add when missionsave is refactored
	CampaignSpecialMode m_mode{CampaignSpecialMode::Loop};
	int m_mainCount{0};
	int m_specCount{0};

	CampaignGraphStyle m_style;

	QRectF m_rect;
	QRectF m_badgeRect;
	qreal m_titleH{18.0};
	qreal m_nameH{24.0};
};

/**
 * Edge between missions.
 * Edges ignore mouse events and are not selectable.
 */
class EdgeItem final : public QObject, public QGraphicsPathItem {
	Q_OBJECT
  public:
	EdgeItem(int missionIndex,
		int branchId,
		bool isSpecial,
		CampaignSpecialMode mode,
		CampaignGraphStyle style,
		QGraphicsItem* parent = nullptr);

	// Standard edge
	void setEndpoints(const QPointF& src, const QPointF& dst, int siblingIndex, int siblingCount);

	// Selfloop variant, wraps outside node rect on the appropriate side
	void setSelfLoop(const QRectF& nodeRectScene, bool sourceIsRightSide, int siblingIndex, int siblingCount);

	// kept for completeness; not used now
	void setSelectedVisual(bool sel);

	// Emphasis state
	enum class Emphasis { Faded, Highlighted };
	void setEmphasis(Emphasis e);
	Emphasis emphasis() const { return m_emphasis; }

	// For focus mode connectivity checks
	int sourceIndex() const { return m_missionIndex; }
	int targetIndex() const { return m_targetIndex; }
	void setTargetIndex(int idx) { m_targetIndex = idx; }

  protected:
	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

  private:
	QPainterPath buildPath(const QPointF& src, const QPointF& dst, int siblingIndex, int siblingCount);
	QPainterPath
	buildSelfLoopPath(const QRectF& nodeRectScene, bool sourceIsRightSide, int siblingIndex, int siblingCount);

	int m_missionIndex{-1};
	int m_branchId{-1};
	bool m_isSpecial{false};
	CampaignSpecialMode m_mode{CampaignSpecialMode::Loop};
	CampaignGraphStyle m_style;

	QColor m_color;
	Qt::PenStyle m_dash{Qt::SolidLine};

	// Cache for arrow drawing
	QPointF m_lastSegmentP1; // second to last point
	QPointF m_lastSegmentP2; // last point

	SCP_vector<QPointF> m_points; // cached polyline points for arrow placement

	Emphasis m_emphasis{Emphasis::Faded};
	int m_targetIndex{-1}; // -1 = unknown/END; else mission index
};

/**
 * End of campaign pill to visualize the "END" target for branches with no next mission.
 */
class EndSinkItem final : public QGraphicsObject {
	Q_OBJECT
  public:
	explicit EndSinkItem(CampaignGraphStyle style, QGraphicsItem* parent = nullptr)
		: QGraphicsObject(parent), m_style(std::move(style))
	{
	}

	QRectF boundingRect() const override
	{
		// Expand upward by port radius so the top port isn't clipped
		QRectF pill(QPointF(0, 0), m_style.endSinkSize);
		return pill.adjusted(-1.0, -m_style.portRadius, +1.0, +1.0);
	}

	void paint(QPainter* p, const QStyleOptionGraphicsItem*, QWidget*) override
	{
		p->setRenderHint(QPainter::Antialiasing, true);

		// draw relative to the pill rect (not the expanded boundingRect)
		const QRectF pill(QPointF(0, 0), m_style.endSinkSize);

		// Inbound port, centered on the pill's top edge
		QPen portPen(m_style.endSinkBorder, 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
		p->setPen(portPen);
		p->setBrush(m_style.inboundPortColor);
		const QPointF portCenter(pill.center().x(), pill.top());
		p->drawEllipse(portCenter, m_style.portRadius, m_style.portRadius);

		// Pill on top
		QPen border(m_style.endSinkBorder, 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
		p->setPen(border);
		p->setBrush(m_style.endSinkFill);
		p->drawRoundedRect(pill, m_style.endSinkRadius, m_style.endSinkRadius);

		// Label
		p->setPen(m_style.endSinkText);
		QFont f = p->font();
		f.setBold(true);
		p->setFont(f);
		p->drawText(pill, Qt::AlignCenter, QStringLiteral("END"));
	}

	// Anchor where inbound edges terminate
	QPointF inboundAnchorScenePos() const
	{
		// Anchor on the pill's top-center
		const QRectF pill(QPointF(0, 0), m_style.endSinkSize);
		return mapToScene(QPointF(pill.center().x(), pill.top()));
	}


  private:
	CampaignGraphStyle m_style;
};

} // namespace detail
