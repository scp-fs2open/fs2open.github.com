#pragma once

#include <QGraphicsObject>
#include <QGraphicsPathItem>
#include <QGraphicsView>
#include <QPointer>
#include <vector>

// Forward declaration to avoid coupling for now
namespace fso::fred::dialogs {
class CampaignEditorDialogModel;
} // namespace fso::fred::dialogs

class QGraphicsScene;

// Forward-declare internal items so we can use pointers before the full namespace block
namespace detail {
class MissionNodeItem;
class EdgeItem;
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
	qreal nubSpacingBottom{6.0};
	qreal nubOffsetX{42.0};

	// Edge routing
	qreal fanoutStart{12.0}; // vertical run from nub before spreading
	qreal fanoutStep{10.0};  // horizontal separation between sibling edges
	qreal edgeWidth{3.0};
	qreal arrowSize{8.0}; // arrowhead size

	// Badge
	qreal badgePad{6.0};
	QSizeF badgeSize{34.0, 18.0}; // pill
	QColor badgeDisabled{180, 180, 180};
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
	// Emitted when an edge is clicked
	void branchSelected(int missionIndex, int branchId);

  protected:
	// Pan/zoom
	void wheelEvent(QWheelEvent* e) override;
	// Grid background
	void drawBackground(QPainter* painter, const QRectF& rect) override;

  private:
	void initScene();
	void drawGrid(QPainter* p, const QRectF& rect);
	void buildMissionNodes();
	void buildMissionEdges();

  private:
	QGraphicsScene* m_scene{nullptr}; // owned by view (parented)
	QPointer<fso::fred::dialogs::CampaignEditorDialogModel> m_model;

	// Centralized visuals
	CampaignGraphStyle m_style;

	// Items we create (aligned to model order)
	std::vector<detail::MissionNodeItem*> m_nodeItems;
	std::vector<detail::EdgeItem*> m_edgeItems;

	bool m_gridVisible{true};
	bool m_zoomEnabled{true};
	qreal m_currentScale{1.0};
	const qreal kMinScale{0.2};
	const qreal kMaxScale{3.0};
};

// ---------- Internal items (Q_OBJECT in header so AUTOMOC runs) ----------

namespace detail {

enum class SpecialMode {
	Loop,
	Fork
};

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
		SpecialMode mode,
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
	SpecialMode m_mode{SpecialMode::Loop};
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
 */
class EdgeItem final : public QObject, public QGraphicsPathItem {
	Q_OBJECT
  public:
	EdgeItem(int missionIndex,
		int branchId,
		bool isSpecial,
		SpecialMode mode,
		const CampaignGraphStyle& style,
		QGraphicsItem* parent = nullptr);

	// Compute and apply a Manhattan path given endpoints and fan-out parameters
	void setEndpoints(const QPointF& src, const QPointF& dst, int siblingIndex, int siblingCount);

	// Colors/pens are selected by ctor args (main vs loop vs fork)
	void setSelectedVisual(bool sel);

  signals:
	void edgeClicked(int missionIndex, int branchId);

  protected:
	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
	void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

  private:
	// Non-const now: updates cached last segment points for arrowhead
	QPainterPath buildPath(const QPointF& src, const QPointF& dst, int siblingIndex, int siblingCount);

  private:
	int m_missionIndex{-1};
	int m_branchId{-1};
	bool m_isSpecial{false};
	SpecialMode m_mode{SpecialMode::Loop};
	const CampaignGraphStyle& m_style;

	QColor m_color;
	Qt::PenStyle m_dash{Qt::SolidLine};

	// Cache for arrow drawing
	QPointF m_lastSegmentP1; // second-to-last point
	QPointF m_lastSegmentP2; // last point (path end)
};

} // namespace detail
