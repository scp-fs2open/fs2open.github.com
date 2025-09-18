#include "ui/widgets/CampaignMissionGraph.h"

#include "mission/missionparse.h"

#include <QScrollBar>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QWheelEvent>
#include <QtMath>
#include <QMenu>
#include <QContextMenuEvent>
#include <cmath>
#include <unordered_map>

using detail::EdgeItem;
using detail::MissionNodeItem;
using detail::CampaignSpecialMode;
using fso::fred::dialogs::CampaignEditorDialogModel;

MissionNodeItem::MissionNodeItem(int missionIndex,
	QString fileLabel,
	QString nameLabel,
	int graphColorRgb,
	CampaignSpecialMode mode,
	int mainBranchCount,
	int specialBranchCount,
	CampaignGraphStyle style,
	QGraphicsItem* parent)
	: QGraphicsObject(parent), m_idx(missionIndex), m_file(std::move(fileLabel)), m_name(std::move(nameLabel)), m_graphColor(graphColorRgb),
	  m_mode(mode), m_mainCount(mainBranchCount), m_specCount(specialBranchCount), m_style(std::move(style))
{
	setFlags(ItemIsSelectable | ItemIsMovable | ItemSendsScenePositionChanges);
	setAcceptHoverEvents(true);
	setCursor(Qt::PointingHandCursor);
	updateGeometry();
}

QRectF MissionNodeItem::boundingRect() const
{
	return m_rect.adjusted(-1, -1, 1, 1);
}

static inline QPainterPath roundedRectPath(const QRectF& r, qreal rad)
{
	QPainterPath path;
	path.addRoundedRect(r, rad, rad);
	return path;
}

void MissionNodeItem::paint(QPainter* p, const QStyleOptionGraphicsItem*, QWidget*)
{
	p->setRenderHint(QPainter::Antialiasing, true);

	// Precompute geometry
	const qreal borderW = isSelected() ? 2.0 : 1.0;
	const QPen borderPen(m_style.nodeBorder, borderW, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

	const qreal topPortY = m_rect.top();       // center exactly on edge; half visible
	const qreal bottomPortY = m_rect.bottom(); // center exactly on edge; half visible
	const qreal centerX = m_rect.center().x();
	const QPointF inboundPort(centerX, topPortY);
	const QPointF mainPort(centerX - m_style.portOffsetX, bottomPortY);
	const QPointF specPort(centerX + m_style.portOffsetX, bottomPortY);

	// Ports first
	p->setPen(borderPen);

	// Top inbound
	p->setBrush(m_style.inboundPortColor);
	p->drawEllipse(inboundPort, m_style.portRadius, m_style.portRadius);

	// Bottom main
	p->setBrush(m_style.standardPortColor);
	p->drawEllipse(mainPort, m_style.portRadius, m_style.portRadius);

	// Bottom special
	const QColor specColor = (m_mode == CampaignSpecialMode::Loop) ? m_style.loopPortColor : m_style.forkPortColor;
	p->setBrush(specColor);
	p->drawEllipse(specPort, m_style.portRadius, m_style.portRadius);

	// Mission Card
	p->setPen(borderPen);
	p->setBrush(m_style.nodeFill);
	p->drawPath(roundedRectPath(m_rect, m_style.nodeRadius));

	// User color stripe
	if (m_graphColor >= 0) {
		const QColor stripe((m_graphColor >> 16) & 0xFF, (m_graphColor >> 8) & 0xFF, (m_graphColor) & 0xFF);
		p->fillRect(
			QRectF(m_rect.left(), m_rect.top() + m_style.padding + m_titleH, m_rect.width(), m_style.stripeHeight),
			stripe);
	}

	// Special Port Type Select Badge
	const bool badgeDisabled = (m_specCount > 0) || !m_style.forksEnabled;
	const QRectF badgeRect(m_rect.right() - m_style.badgePad - m_style.badgeSize.width(),
		m_rect.top() + m_style.badgePad,
		m_style.badgeSize.width(),
		m_style.badgeSize.height());
	m_badgeRect = badgeRect;

	QColor badgeCol = (m_mode == CampaignSpecialMode::Loop) ? m_style.loopPortColor : m_style.forkPortColor;
	if (badgeDisabled)
		badgeCol = m_style.badgeDisabled;

	p->setBrush(badgeCol);
	p->setPen(Qt::NoPen);
	p->drawRoundedRect(badgeRect, badgeRect.height() / 2.0, badgeRect.height() / 2.0);

	// Icon placeholder // TODO: replace with real icon
	p->setPen(QPen(Qt::white, 2));
	if (m_mode == CampaignSpecialMode::Loop) {
		const QRectF arcRect = badgeRect.adjusted(4, 2, -4, -2);
		p->drawArc(arcRect, 45 * 16, 270 * 16);
		p->drawLine(QPointF(arcRect.center().x(), arcRect.top() + 2),
			QPointF(arcRect.center().x() + 5, arcRect.top() + 6));
	} else {
		const QPointF c = badgeRect.center();
		p->drawLine(QPointF(c.x(), badgeRect.top() + 3), QPointF(c.x(), badgeRect.bottom() - 3));
		p->drawLine(QPointF(c.x(), badgeRect.top() + 3), QPointF(c.x() - 6, c.y() - 3));
		p->drawLine(QPointF(c.x(), badgeRect.top() + 3), QPointF(c.x() + 6, c.y() - 3));
	}

	// Text: filename and mission name
	p->setPen(Qt::black);
	QFont f = p->font();
	f.setPointSizeF(f.pointSizeF() - 1);
	p->setFont(f);
	const QRectF fileRect(m_rect.left() + m_style.padding,
		m_rect.top() + m_style.padding,
		m_rect.width() - 2 * m_style.padding - m_style.badgeSize.width() - m_style.badgePad * 2,
		m_titleH);
	p->drawText(fileRect, Qt::AlignVCenter | Qt::AlignLeft | Qt::TextSingleLine, m_file);

	QFont f2 = p->font();
	f2.setPointSizeF(f.pointSizeF() + 2);
	f2.setBold(true);
	p->setFont(f2);

	const qreal nameTop = fileRect.bottom() + m_style.stripeHeight + m_style.padding;
	const QRectF nameRect(m_rect.left() + m_style.padding, nameTop, m_rect.width() - 2 * m_style.padding, m_nameH);
	const QString nameToDraw = (m_name.isEmpty() || m_name == m_file) ? m_file : m_name;
	p->drawText(nameRect, Qt::AlignTop | Qt::AlignLeft | Qt::TextWordWrap, nameToDraw);
}

void MissionNodeItem::mousePressEvent(QGraphicsSceneMouseEvent* e)
{
	if (m_badgeRect.contains(e->pos())) {
		if (m_specCount == 0) {
			if (m_style.forksEnabled) {
				Q_EMIT specialModeToggleRequested(m_idx);
			}
			e->accept();
			return;
		}
	}
	QGraphicsObject::mousePressEvent(e);
}

void MissionNodeItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* e)
{
	if (e->button() == Qt::LeftButton) {
		Q_EMIT missionSelected(m_idx);
	}
	QGraphicsObject::mouseReleaseEvent(e);
}

QVariant MissionNodeItem::itemChange(GraphicsItemChange change, const QVariant& value)
{
	// Snap while dragging; emit final position after it changes
	if (change == ItemPositionChange) {
		const QPointF p = value.toPointF();
		const qreal s = m_style.minorStep;
		const QPointF snapped(std::round(p.x() / s) * s, std::round(p.y() / s) * s);
		return snapped;
	}
	if (change == ItemPositionHasChanged) {
		// Emit scenespace topleft for persistence
		Q_EMIT nodeMoved(m_idx, mapToScene(QPointF(0, 0)));
	}
	return QGraphicsObject::itemChange(change, value);
}

void MissionNodeItem::updateGeometry()
{
	m_titleH = 18.0;
	m_nameH = 24.0;
	m_rect = QRectF(0, 0, 240.0, 120.0);
}

detail::MissionNodeItem::Port detail::MissionNodeItem::hitTestPortScene(const QPointF& sp) const
{
	// If this item is no longer in a scene, treat as no port (avoids mapToScene on a dying item)
	if (!scene())
		return Port::None;
	
	const qreal r = m_style.portRadius + m_style.portHitExtra; // configurable hit radius
	auto near = [&](const QPointF& a, const QPointF& b) { return QLineF(a, b).length() <= r; };

	const QPointF inP = inboundPortScenePos();
	const QPointF mainP = mainPortScenePos();
	const QPointF specP = specialPortScenePos();

	if (near(sp, mainP))
		return Port::Main;
	if (near(sp, specP))
		return Port::Special;
	if (near(sp, inP))
		return Port::Inbound;
	return Port::None;
}

QPointF MissionNodeItem::inboundPortScenePos() const
{
	return mapToScene(QPointF(m_rect.center().x(), m_rect.top()));
}

QPointF MissionNodeItem::mainPortScenePos() const
{
	return mapToScene(QPointF(m_rect.center().x() - m_style.portOffsetX, m_rect.bottom()));
}

QPointF MissionNodeItem::specialPortScenePos() const
{
	return mapToScene(QPointF(m_rect.center().x() + m_style.portOffsetX, m_rect.bottom()));
}

EdgeItem::EdgeItem(int missionIndex,
	int branchId,
	bool isSpecial,
	CampaignSpecialMode mode,
	CampaignGraphStyle style,
	QGraphicsItem* parent)
	: QGraphicsPathItem(parent), m_missionIndex(missionIndex), m_branchId(branchId), m_isSpecial(isSpecial),
	  m_mode(mode), m_style(std::move(style))
{
	setZValue(5); // below nodes (nodes use 10)
	// noninteractive
	setAcceptedMouseButtons(Qt::NoButton);
	setAcceptHoverEvents(false);

	// color / dash
	if (!m_isSpecial) {
		m_color = m_style.standardPortColor;
		m_dash = Qt::SolidLine;
	} else {
		if (m_mode == CampaignSpecialMode::Loop) {
			m_color = m_style.loopPortColor;
			m_dash = Qt::DashLine;
		} else {
			m_color = m_style.forkPortColor;
			m_dash = Qt::DashDotLine;
		}
	}
	QPen pen(m_color, m_style.edgeWidth, m_dash, Qt::RoundCap, Qt::RoundJoin);
	pen.setCosmetic(true);
	setPen(pen);
}

void EdgeItem::setSelectedVisual(bool sel)
{
	// kept for future use
	QPen pen = this->pen();
	pen.setWidthF(sel ? m_style.edgeWidth + 1.5 : m_style.edgeWidth);
	setPen(pen);
	update();
}

void EdgeItem::setEmphasis(Emphasis e)
{
	m_emphasis = e;

	// Drive visual emphasis via item opacity and a small Z tweak so highlights sit above fades
	const qreal op = (e == Emphasis::Highlighted) ? m_style.highlightedEdgeOpacity : m_style.fadedEdgeOpacity;
	setOpacity(op);

	// Keep edges under nodes; just separate the two edge groups a bit
	const qreal baseZ = 5.0; // keep consistent with existing z for edges
	setZValue(baseZ + ((e == Emphasis::Highlighted) ? 1.0 : 0.0));
}

void EdgeItem::setEndpoints(const QPointF& src, const QPointF& dst, int siblingIndex, int siblingCount)
{
	const auto path = buildPath(src, dst, siblingIndex, siblingCount);
	setPath(path);
	update();
}

void EdgeItem::setSelfLoop(const QRectF& nodeRectScene, bool sourceIsRightSide, int siblingIndex, int siblingCount)
{
	const auto path = buildSelfLoopPath(nodeRectScene, sourceIsRightSide, siblingIndex, siblingCount);
	setPath(path);
	update();
}

QPainterPath EdgeItem::buildPath(const QPointF& src, const QPointF& dst, int siblingIndex, int siblingCount)
{
	// Sibling separation centered around zero
	const qreal sibOffset = (siblingCount > 1) ? ((siblingIndex - (siblingCount - 1) * 0.5) * m_style.fanoutStep) : 0.0;

	// Decide horizontal direction at the source:
	//  - If enabled, head toward the target's X so we don't jog the wrong way.
	//  - Otherwise, keep the legacy main-left/special-right feel.
	qreal dirX = +1.0;
	if (m_style.outboundDirectionTowardTarget) {
		dirX = (dst.x() - src.x() >= 0.0) ? +1.0 : -1.0;
	} else {
		dirX = m_isSpecial ? +1.0 : -1.0;
	}

	// Per type drop/jog
	const qreal typeDrop = m_isSpecial ? m_style.outboundDropSpecial : m_style.outboundDropMain;
	const qreal typeJogX = m_isSpecial ? m_style.outboundJogSpecial : m_style.outboundJogMain;

	const qreal drop =
		m_style.outboundApproachEnabled ? std::max<qreal>(m_style.fanoutStart, typeDrop) : m_style.fanoutStart;

	const qreal baseJogX = m_style.outboundApproachEnabled ? typeJogX : 0.0;

	// Source side: p0..p2
	const QPointF p0 = src;                                            // at port
	const QPointF p1 = p0 + QPointF(0, drop);                          // vertical drop
	const QPointF p2 = p1 + QPointF((sibOffset + baseJogX) * dirX, 0); // horizontal jog + sibling fanout

	// Target side
	const qreal inset = std::max<qreal>(2.0, m_style.arrowTargetInset);

	QPointF p3, p4, p5;
	if (m_style.inboundApproachEnabled) {
		const qreal dx = dst.x() - p2.x();
		const qreal dirInX = (dx >= 0.0) ? +1.0 : -1.0;
		const qreal jogX = m_style.inboundApproachJog;
		const qreal rise = std::max<qreal>(m_style.fanoutStart, m_style.inboundApproachRise);

		const qreal jogXPos = dst.x() - dirInX * jogX;
		p3 = QPointF(jogXPos, p2.y());
		p4 = QPointF(jogXPos, dst.y() - rise);
		p5 = QPointF(dst.x(), p4.y());
	} else {
		p3 = QPointF(dst.x(), p2.y());
		p4 = QPointF(dst.x(), dst.y() - m_style.fanoutStart);
		p5 = p4;
	}

	// Final short vertical into the arrow tip, stop short so the arrowhead is visible
	const QPointF p6 = dst - QPointF(0, inset);

	QPainterPath path(p0);
	path.lineTo(p1);
	path.lineTo(p2);
	path.lineTo(p3);
	path.lineTo(p4);
	if (m_style.inboundApproachEnabled) {
		path.lineTo(p5);
	}
	path.lineTo(p6);

	// Cache for arrow at the end + interval arrows
	m_lastSegmentP1 = m_style.inboundApproachEnabled ? p5 : p4;
	m_lastSegmentP2 = p6;

	if (m_style.inboundApproachEnabled) {
		m_points = {p0, p1, p2, p3, p4, p5, p6};
	} else {
		m_points = {p0, p1, p2, p3, p4, p6};
	}

	return path;
}

QPainterPath EdgeItem::buildSelfLoopPath(const QRectF& node, bool sourceIsRightSide, int siblingIndex, int siblingCount)
{
	// Offset spread for multiple selfloops
	const qreal spread =
		(siblingCount > 1) ? ((siblingIndex - (siblingCount - 1) * 0.5) * m_style.selfLoopSpread) : 0.0;

	const qreal sideX = sourceIsRightSide ? (node.right() + m_style.selfLoopMargin + std::abs(spread))
										  : (node.left() - m_style.selfLoopMargin - std::abs(spread));

	const QPointF src = sourceIsRightSide ? QPointF(node.center().x() + m_style.portOffsetX, node.bottom())
										  : QPointF(node.center().x() - m_style.portOffsetX, node.bottom());

	const QPointF dst(node.center().x(), node.top());

	// Rectangular wrap outside node bounds
	const qreal typeDrop = m_isSpecial ? m_style.outboundDropSpecial : m_style.outboundDropMain;
	const qreal drop = m_style.outboundApproachEnabled ? std::max<qreal>(m_style.fanoutStart, typeDrop) : m_style.fanoutStart;

	const QPointF p0 = src;
	const QPointF p1 = p0 + QPointF(0, drop);
	const QPointF p2(sideX, p1.y());
	const QPointF p3(sideX, node.top() - m_style.fanoutStart);

	// Stop short of inbound port so arrow is visible
	const qreal inset = std::max<qreal>(2.0, m_style.arrowTargetInset);
	const QPointF p4(node.center().x(), p3.y());
	const QPointF p5 = dst - QPointF(0, inset);

	QPainterPath path(p0);
	path.lineTo(p1);
	path.lineTo(p2);
	path.lineTo(p3);
	path.lineTo(p4);
	path.lineTo(p5);

	m_lastSegmentP1 = p4;
	m_lastSegmentP2 = p5;

	// Full point list for arrow placement
	m_points = {p0, p1, p2, p3, p4, p5};

	return path;
}

void EdgeItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	// Draw the line/path first
	QGraphicsPathItem::paint(painter, option, widget);

	// Helper to draw a single arrow at tip pointing along dir
	auto drawArrowAt = [&](const QPointF& tip, const QPointF& dir, qreal size) {
		if (size <= 0.0)
			return;
		const QPointF ortho(-dir.y(), dir.x());
		QPolygonF arrow;
		arrow << tip << (tip - dir * size + ortho * (size * 0.5)) << (tip - dir * size - ortho * (size * 0.5));
		QBrush brush(this->pen().color());
		QPen pen = this->pen();
		pen.setCapStyle(Qt::SquareCap);
		painter->setPen(pen);
		painter->setBrush(brush);
		painter->drawPolygon(arrow);
	};

	// Arrow at the very end
	{
		const QPointF a = m_lastSegmentP1;
		const QPointF b = m_lastSegmentP2;
		const QPointF v = b - a;
		const qreal len = std::hypot(v.x(), v.y());
		if (len > 0.001) {
			drawArrowAt(b, v / len, m_style.arrowSize);
		}
	}

	// Arrows at regular intervals along each straight segment
	if (m_points.size() < 2)
		return;

	const qreal interval = std::max<qreal>(12.0, m_style.arrowInterval);
	const qreal endMargin = std::max<qreal>(m_style.arrowSize + 2.0, 10.0); // keep away from corners/ends

	for (size_t i = 0; i + 1 < m_points.size(); ++i) {
		const QPointF p1 = m_points[i];
		const QPointF p2 = m_points[i + 1];
		const QPointF seg = p2 - p1;
		const qreal segLen = std::hypot(seg.x(), seg.y());
		if (segLen <= endMargin)
			continue;

		const QPointF dir = seg / segLen;

		const qreal start = std::max(interval, endMargin);
		for (qreal d = start; d <= segLen - endMargin; d += interval) {
			const QPointF pos = p1 + dir * d;
			drawArrowAt(pos, dir, m_style.arrowSize);
		}
	}
}

CampaignMissionGraph::CampaignMissionGraph(QWidget* parent) : QGraphicsView(parent)
{
	initScene();

	// View behavior
	setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
	setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
	setDragMode(QGraphicsView::ScrollHandDrag);
	setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
	setBackgroundBrush(m_style.bgColor);
	setFrameShape(QFrame::NoFrame);

	// Ensure when the scene is smaller than the viewport it sits at the topleft,
	// and when we reset the scene rect we can scroll to (0,0).
	setAlignment(Qt::AlignLeft | Qt::AlignTop);
}

void CampaignMissionGraph::onNodeMoved(int missionIndex, QPointF sceneTopLeft)
{
	if (!m_model)
		return;
	// Persist snapped scene position back into the model
	m_model->setMissionGraphX(missionIndex, static_cast<int>(std::lround(sceneTopLeft.x())));
	m_model->setMissionGraphY(missionIndex, static_cast<int>(std::lround(sceneTopLeft.y())));

	// Rebuild only edges and keep the current viewport
	rebuildEdgesOnly();

	// Expand scene rect if needed but don't autoscroll to topleft
	updateSceneRectToContent(/*scrollToTopLeft=*/false);
}

void CampaignMissionGraph::onSceneSelectionChanged()
{
	// Ignore signals we caused ourselves
	if (m_internallySelecting)
		return;
	if (!m_scene)
		return;

	// Find the first selected mission node
	int idx = -1;
	const auto items = m_scene->selectedItems();
	for (QGraphicsItem* gi : items) {
		if (auto* n = qgraphicsitem_cast<detail::MissionNodeItem*>(gi)) {
			idx = n->missionIndex();
			break;
		}
	}

	// Update internal selection and emphasis without tweaking scene selection again
	m_selectedMissionIndex = idx;
	applyFocusEmphasis();

	Q_EMIT missionSelected(idx);
}

void CampaignMissionGraph::initScene()
{
	m_scene = new QGraphicsScene(this);
	setScene(m_scene);
}

static inline bool nearlyEqual(qreal a, qreal b, qreal eps = 0.5)
{
	return std::abs(a - b) <= eps;
}
static inline bool rectNearlyEqual(const QRectF& a, const QRectF& b, qreal eps = 0.5)
{
	return nearlyEqual(a.left(), b.left(), eps) && nearlyEqual(a.top(), b.top(), eps) &&
		   nearlyEqual(a.right(), b.right(), eps) && nearlyEqual(a.bottom(), b.bottom(), eps);
}

void CampaignMissionGraph::updateSceneRectToContent(bool scrollToTopLeft)
{
	if (!m_scene || m_updatingSceneRect)
		return;

	QRectF itemsRect = m_scene->itemsBoundingRect();
	if (!itemsRect.isValid() || itemsRect.isEmpty()) {
		itemsRect = QRectF(0, 0, m_style.nodeSize.width() * 3.0, m_style.nodeSize.height() * 2.0);
	}

	// Proposed rect = content margins
	QRectF proposed = itemsRect.adjusted(-m_style.contentMarginX,
		-m_style.contentMarginY,
		+m_style.contentMarginX,
		+m_style.contentMarginY);

	// Avoid endless churn
	const QRectF current = m_scene->sceneRect();
	if (rectNearlyEqual(proposed, current)) {
		// Still optionally scroll on first build
		if (scrollToTopLeft) {
			setAlignment(Qt::AlignLeft | Qt::AlignTop);
			if (horizontalScrollBar())
				horizontalScrollBar()->setValue(horizontalScrollBar()->minimum());
			if (verticalScrollBar())
				verticalScrollBar()->setValue(verticalScrollBar()->minimum());
		}
		return;
	}

	// During interactive moves we only want to grow the rect, not shrink it
	bool interactiveMove = m_drag.active || (scene() && scene()->mouseGrabberItem());
	if (interactiveMove) {
		// Expand each side to at least the current extents
		proposed.setLeft(std::min(proposed.left(), current.left()));
		proposed.setTop(std::min(proposed.top(), current.top()));
		proposed.setRight(std::max(proposed.right(), current.right()));
		proposed.setBottom(std::max(proposed.bottom(), current.bottom()));
		
		if (rectNearlyEqual(proposed, current))
			return;
	}

	// Reentrancy guard
	m_updatingSceneRect = true;
	m_scene->setSceneRect(proposed);
	m_updatingSceneRect = false;

	if (scrollToTopLeft) {
		setAlignment(Qt::AlignLeft | Qt::AlignTop);
		if (horizontalScrollBar())
			horizontalScrollBar()->setValue(horizontalScrollBar()->minimum());
		if (verticalScrollBar())
			verticalScrollBar()->setValue(verticalScrollBar()->minimum());
	}
}

void CampaignMissionGraph::setModel(CampaignEditorDialogModel* model)
{
	m_model = model;
	rebuildAll(true);
}

void CampaignMissionGraph::rebuildAll(bool refocus)
{
	if (!m_scene)
		return;

	ensureSceneHooks();

	// Clean up END sink explicitly to avoid dangling pointer across clears
	if (m_endSink) {
		m_scene->removeItem(m_endSink);
		delete m_endSink.data();
		m_endSink = nullptr;
	}

	// Reset scene content
	m_scene->clear();
	m_nodeItems.clear();
	m_edgeItems.clear();

	if (!m_model) {
		// Even with no model, give a small usable scene area
		updateSceneRectToContent(refocus);
		return;
	}

	buildMissionNodes();
	buildMissionEdges();

	// Size scene to the items and put the view at the top-left
	updateSceneRectToContent(refocus);

	applyFocusEmphasis();
}

void CampaignMissionGraph::setSelectedMission(int missionIndex, bool makeVisible, bool centerOnItem)
{
	if (!m_scene)
		return;

	m_internallySelecting = true; // guard scene signal while we change selection

	// Clear any previous selection first
	m_scene->clearSelection();

	// Track selection for focus mode
	m_selectedMissionIndex = (missionIndex >= 0 && missionIndex < static_cast<int>(m_nodeItems.size())) ? missionIndex : -1;

	// Apply selection to the item, if in range
	if (m_selectedMissionIndex >= 0) {
		auto* item = m_nodeItems[m_selectedMissionIndex];
		if (item)
			item->setSelected(true);

		if (makeVisible && item) {
			QGraphicsView::ensureVisible(item, 40, 40);
		}
		if (centerOnItem && item) {
			this->centerOn(item);
		}
	}

	m_internallySelecting = false; // done with programmatic change

	// Recompute edge emphasis after every selection change
	applyFocusEmphasis();

	Q_EMIT missionSelected(missionIndex);
}

void CampaignMissionGraph::clearSelectedMission()
{
	if (!m_scene)
		return;
	m_internallySelecting = true;
	m_scene->clearSelection();
	m_internallySelecting = false;

	m_selectedMissionIndex = -1;
	applyFocusEmphasis();
	Q_EMIT missionSelected(-1);
}

void CampaignMissionGraph::buildMissionNodes()
{
	const auto& missions = m_model->getCampaignMissions();
	const QPointF fallbackStep(m_style.layoutStepX, m_style.layoutStepY);

	m_nodeItems.reserve(missions.size());

	for (int i = 0; i < static_cast<int>(missions.size()); ++i) {
		const auto& m = missions[i];

		// Position
		QPointF pos;
		if (m.graph_x != INT_MIN && m.graph_y != INT_MIN) {
			pos = QPointF(m.graph_x, m.graph_y);
		} else {
			pos = QPointF(m.position * fallbackStep.x(), m.level * fallbackStep.y());
		}

		// Branch counts and special mode
		int mainCount = 0, specCount = 0;
		for (const auto& b : m.branches) {
			(b.is_loop || b.is_fork) ? ++specCount : ++mainCount;
		}
		const auto mode = m.special_mode_hint;

		// Labels
		QString fileLabel = QString::fromStdString(m.filename);
		QString nameLabel;

		mission mission_info;
		if (get_mission_info(m.filename.c_str(), &mission_info) == 0) { //TODO make this a model getter and remove missionparse.h include
			nameLabel = QString::fromStdString(mission_info.name);
		}

		auto* item = new MissionNodeItem(i, fileLabel, nameLabel, m.graph_color, mode, mainCount, specCount, m_style);
		item->setPos(pos);
		item->setZValue(10);

		connect(item, &MissionNodeItem::missionSelected, this, &CampaignMissionGraph::missionSelected);
		connect(item,
			&MissionNodeItem::specialModeToggleRequested,
			this,
			&CampaignMissionGraph::specialModeToggleRequested);
		connect(item, &MissionNodeItem::nodeMoved, this, &CampaignMissionGraph::onNodeMoved);
		connect(item, &detail::MissionNodeItem::missionSelected, this, [this](int idx) {
			setSelectedMission(idx, /*makeVisible=*/true, /*centerOnItem=*/false);
		});

		m_scene->addItem(item);
		m_nodeItems.push_back(item);
	}
}

void CampaignMissionGraph::buildMissionEdges()
{
	const auto& missions = m_model->getCampaignMissions();
	if (missions.empty())
		return;

	// Ensure END sink exists/positioned if any MAIN branch uses it
	ensureEndSink();

	// Map mission name to index for branch targets
	std::unordered_map<std::string, int> nameToIndex;
	nameToIndex.reserve(missions.size());
	for (int i = 0; i < static_cast<int>(missions.size()); ++i) {
		nameToIndex[missions[i].filename] = i;
	}

	// For each mission, build edges to other missions and self if the feature is active
	for (int i = 0; i < static_cast<int>(missions.size()); ++i) {
		const auto& m = missions[i];
		const auto* srcNode = m_nodeItems[i];

		const auto mode = m.special_mode_hint;

		// Prepare sibling counters for fanout
		int mainTotal = 0, specTotal = 0;
		for (const auto& b : m.branches) {
			(b.is_loop || b.is_fork) ? ++specTotal : ++mainTotal;
		}
		int mainIdx = 0, specIdx = 0;

		for (const auto& b : m.branches) {
			// Normal MAIN branch to END sink if no target
			if (!b.is_loop && !b.is_fork && b.next_mission_name.empty()) {
				if (m_endSink) {
					const int sibCount = mainTotal;
					const int sibIndex = mainIdx++;

					const QPointF srcPt = srcNode->mainPortScenePos();
					const QPointF dstPt = m_endSink->inboundAnchorScenePos();

					auto* edge = new detail::EdgeItem(i, b.id, /*isSpecial*/ false, mode, m_style);
					edge->setTargetIndex(-1);
					edge->setEndpoints(srcPt, dstPt, sibIndex, sibCount);
					m_scene->addItem(edge);
					m_edgeItems.push_back(edge);
				}
				continue; // handled
			}

			// SPECIAL with empty target is nonsensical; ignore
			if ((b.is_loop || b.is_fork) && b.next_mission_name.empty()) {
				// Error checker should warn about this if it doesn't
				continue;
			}

			auto it = nameToIndex.find(b.next_mission_name);
			if (it == nameToIndex.end())
				continue;

			const int j = it->second;
			const auto* dstNode = m_nodeItems[j];
			if (!srcNode || !dstNode)
				continue;

			const bool isSpecial = (b.is_loop || b.is_fork);
			const int sibCount = isSpecial ? specTotal : mainTotal;
			const int sibIndex = isSpecial ? specIdx++ : mainIdx++;

			auto* edge = new EdgeItem(i, b.id, isSpecial, mode, m_style);
			edge->setTargetIndex(j);

			if (i == j) {
				// Selfloop: draw outside node
				if (m_style.showSelfLoops) {
					const QRectF nodeRectScene = srcNode->mapRectToScene(srcNode->boundingRect());
					const bool sourceRight = isSpecial; // special port on right; main on left
					edge->setSelfLoop(nodeRectScene, sourceRight, sibIndex, sibCount);
					m_scene->addItem(edge);
					m_edgeItems.push_back(edge);
				} else {
					delete edge;
				}
				continue;
			}

			// Normal edge
			const QPointF srcPt = isSpecial ? srcNode->specialPortScenePos() : srcNode->mainPortScenePos();
			const QPointF dstPt = dstNode->inboundPortScenePos();

			edge->setEndpoints(srcPt, dstPt, sibIndex, sibCount);

			m_scene->addItem(edge);
			m_edgeItems.push_back(edge);
		}
	}

	applyFocusEmphasis();
}

void CampaignMissionGraph::ensureEndSink()
{
	// Needs END only if any mission node exists
	bool needsEnd = false;
	if (m_model) {
		const auto& missions = m_model->getCampaignMissions();
		needsEnd = !missions.empty();
	}

	// Remove if not needed
	if (!needsEnd) {
		if (m_endSink) {
			m_scene->removeItem(m_endSink);
			delete m_endSink;
			m_endSink = nullptr;
		}
		return;
	}

	// Create if missing
	if (!m_endSink) {
		m_endSink = new detail::EndSinkItem(m_style);
		m_endSink->setZValue(9.0);
		m_scene->addItem(m_endSink);
	}

	// Position below the union of mission nodes
	// Future TODO: Let the user move this around. Will require being able to save it's position to the campaign file
	if (!m_nodeItems.empty()) {
		QRectF nodesRect = m_nodeItems.front()->mapRectToScene(m_nodeItems.front()->boundingRect());
		for (size_t i = 1; i < m_nodeItems.size(); ++i) {
			nodesRect = nodesRect.united(m_nodeItems[i]->mapRectToScene(m_nodeItems[i]->boundingRect()));
		}
		const qreal x = nodesRect.center().x() - m_style.endSinkSize.width() * 0.5;
		const qreal y = nodesRect.bottom() + m_style.endSinkMargin;
		m_endSink->setPos(QPointF(x, y));
	}
}

void CampaignMissionGraph::ensureSceneHooks()
{
	if (!m_scene || m_sceneHooksInstalled)
		return;
	connect(m_scene, &QGraphicsScene::selectionChanged, this, &CampaignMissionGraph::onSceneSelectionChanged);
	m_sceneHooksInstalled = true;
}

void CampaignMissionGraph::rebuildEdgesOnly()
{
	// Remove existing edges
	for (auto* e : m_edgeItems) {
		m_scene->removeItem(e);
		delete e;
	}
	m_edgeItems.clear();

	// Reposition/create END sink
	ensureEndSink();

	// Rebuild edges using current node positions and model branches
	buildMissionEdges();
}

bool CampaignMissionGraph::hasRepeatBranch(int missionIndex) const
{
	if (!m_model)
		return false;
	const auto& missions = m_model->getCampaignMissions();
	if (!SCP_vector_inbounds(missions, missionIndex))
		return false;

	const auto& m = missions[missionIndex];
	const auto& selfName = m.filename;

	return std::any_of(m.branches.begin(), m.branches.end(), [&](const auto& b) {
		return !b.is_loop && !b.is_fork && b.next_mission_name == selfName;
	});
}


detail::MissionNodeItem* CampaignMissionGraph::nodeAtScenePos(const QPointF& scenePt) const
{
	if (!m_scene)
		return nullptr;

	// Items returns top most first; skip anything not actually in this scene anymore.
	const auto itemsHere = m_scene->items(scenePt);
	for (QGraphicsItem* gi : itemsHere) {
		if (!gi || gi->scene() != m_scene)
			continue; // being removed or belongs elsewhere
		if (auto* n = qgraphicsitem_cast<detail::MissionNodeItem*>(gi)) {
			return n;
		}
	}
	return nullptr;
}

bool CampaignMissionGraph::tryFinishConnectionAt(const QPointF& scenePt)
{
	if (!m_model || m_drag.fromIndex < 0)
		return false;

	const qreal hitR = m_style.portRadius + m_style.portHitExtra;

	// END sink
	if (!m_drag.isSpecial && m_endSink) {
		const QPointF anchor = m_endSink->inboundAnchorScenePos();
		if (QLineF(scenePt, anchor).length() <= hitR) {
			m_model->addEndBranch(m_drag.fromIndex);
			return true;
		}
	}

	// Mission inbound port
	if (auto* dst = nodeAtScenePos(scenePt)) {
		const QPointF anchor = dst->inboundPortScenePos();
		if (QLineF(scenePt, anchor).length() <= hitR) {
			const int toIdx = dst->missionIndex();
			if (m_drag.isSpecial) {
				m_model->addSpecialBranch(m_drag.fromIndex, toIdx);
			} else {
				m_model->addBranch(m_drag.fromIndex, toIdx);
			}
			return true;
		}
	}

	// Empty space: ask dialog to create a new mission here and connect
	// Snap drop to grid; use it as the node's top-left
	const qreal s = m_style.minorStep;
	const QPointF snapped(qRound(scenePt.x() / s) * s, qRound(scenePt.y() / s) * s);

	// Don't spawn if we actually clicked on an item's body
	if (!nodeAtScenePos(scenePt) && !(m_endSink && m_endSink->sceneBoundingRect().contains(scenePt))) {
		m_spawnPending = true;
		Q_EMIT createMissionAtAndConnectRequested(snapped, m_drag.fromIndex, m_drag.isSpecial);
		// return false so caller won't rebuild; dialog will rebuild after it adds the mission
		return false;
	}

	return false;
}

void CampaignMissionGraph::cancelDrag()
{
	// If a rebuild cleared the scene, QPointer auto nulls and this will noop safely
	if (m_drag.preview) {
		if (m_drag.preview->scene()) {
			m_drag.preview->scene()->removeItem(m_drag.preview);
		}
		delete m_drag.preview.data();
	}

	m_drag = DragState{}; // resets flags and clears the QPointer
}

void CampaignMissionGraph::applyFocusEmphasis()
{
	if (!m_style.focusModeEnabled) {
		// Focus mode off: everything highlighted
		for (auto* e : m_edgeItems) {
			if (!e)
				continue;
			e->setEmphasis(detail::EdgeItem::Emphasis::Highlighted);
		}
		return;
	}

	const int sel = m_selectedMissionIndex;

	for (auto* e : m_edgeItems) {
		if (!e)
			continue;

		bool connected = false;
		if (sel >= 0) {
			if (m_style.focusCountOutgoing && e->sourceIndex() == sel)
				connected = true;
			if (m_style.focusCountIncoming && e->targetIndex() == sel)
				connected = true;
		} else {
			// No selection means everything de-emphasized
			connected = false;
		}

		e->setEmphasis(connected ? detail::EdgeItem::Emphasis::Highlighted : detail::EdgeItem::Emphasis::Faded);
	}
}

void CampaignMissionGraph::zoomToFitAll(qreal margin)
{
	if (!m_scene || m_scene->items().isEmpty())
		return;
	const QRectF itemsRect = m_scene->itemsBoundingRect().adjusted(-margin, -margin, margin, margin);
	if (!itemsRect.isEmpty()) {
		fitInView(itemsRect, Qt::KeepAspectRatio);
		m_currentScale = transform().m11();
	}
}

void CampaignMissionGraph::setGridVisible(bool on)
{
	if (m_gridVisible == on)
		return;
	m_gridVisible = on;
	viewport()->update();
}

void CampaignMissionGraph::wheelEvent(QWheelEvent* e)
{
	if (!m_zoomEnabled) {
		QGraphicsView::wheelEvent(e);
		return;
	}

	const QPoint numDeg = e->angleDelta();
	if (numDeg.isNull()) {
		e->ignore();
		return;
	}

	const qreal step = (numDeg.y() > 0) ? 1.10 : (1.0 / 1.10);
	const qreal next = qBound(kMinScale, m_currentScale * step, kMaxScale);
	const qreal factor = next / m_currentScale;

	if (!qFuzzyCompare(factor, 1.0)) {
		scale(factor, factor);
		m_currentScale = next;
	}
	e->accept();
}

void CampaignMissionGraph::drawBackground(QPainter* p, const QRectF& rect)
{
	if (!m_gridVisible)
		return;

	p->save();
	p->setRenderHint(QPainter::Antialiasing, false);

	// Fill the currently exposed area
	p->fillRect(rect, m_style.bgColor);

	// Draw grid over the same scene rect
	drawGrid(p, rect);

	p->restore();
}

void CampaignMissionGraph::mousePressEvent(QMouseEvent* ev)
{
	const QPointF sp = mapToScene(ev->pos());

	// If we're already dragging, ignore
	if (!m_drag.active) {
		if (auto* raw = nodeAtScenePos(sp)) {
			QPointer<detail::MissionNodeItem> n(raw);

			// If the item is already gone, bail safely
			if (!n) {
				ev->ignore();
				return;
			}

			const auto hit = n->hitTestPortScene(sp);
			if (hit == detail::MissionNodeItem::Port::Main || hit == detail::MissionNodeItem::Port::Special) {
				// begin drag
				m_drag.active = true;
				m_drag.isSpecial = (hit == detail::MissionNodeItem::Port::Special);

				// Recheck pointer right before using it again
				if (!n) {
					ev->ignore();
					return;
				}

				m_drag.fromIndex = n->missionIndex();
				m_drag.srcPt = m_drag.isSpecial ? n->specialPortScenePos() : n->mainPortScenePos();

				// Build preview edge
				const auto& missions = m_model->getCampaignMissions();
				const auto mode = missions[m_drag.fromIndex].special_mode_hint;

				m_drag.preview = new detail::EdgeItem(m_drag.fromIndex, /*branchId*/ -1, m_drag.isSpecial, mode, m_style);
				m_drag.preview->setZValue(6.0);
				m_drag.preview->setEndpoints(m_drag.srcPt, sp, /*sibIndex*/ 0, /*sibCount*/ 1);
				m_drag.preview->setEmphasis(detail::EdgeItem::Emphasis::Highlighted);
				m_scene->addItem(m_drag.preview);

				// Disable hand drag while connecting
				setDragMode(QGraphicsView::NoDrag);
				ev->accept();
				return;
			}
		}
	}

	// not starting a connect; fall back to normal behavior
	QGraphicsView::mousePressEvent(ev);
}

void CampaignMissionGraph::mouseReleaseEvent(QMouseEvent* ev)
{
	if (m_drag.active) {
		const QPointF sp = mapToScene(ev->pos());
		bool made = tryFinishConnectionAt(sp);

		cancelDrag();
		setDragMode(QGraphicsView::ScrollHandDrag);

		if (made) {
			// Edges reflect the new branch; keep viewport stable
			rebuildEdgesOnly();
			updateSceneRectToContent(/*scrollToTopLeft=*/false);
		} else if (m_spawnPending) {
			// Dialog will add mission & rebuild; just clear the flag.
			m_spawnPending = false;
		}
		ev->accept();
		return;
	}
	QGraphicsView::mouseReleaseEvent(ev);
}

void CampaignMissionGraph::contextMenuEvent(QContextMenuEvent* e)
{
	const QPointF sp = mapToScene(e->pos());

	// Node under cursor?
	if (auto* n = nodeAtScenePos(sp)) {
		const int idx = n->missionIndex();

		QMenu menu(this);
		QAction* actSetFirst = menu.addAction(tr("Set as First Mission"));
		QAction* actDelete = menu.addAction(tr("Delete mission"));
		QAction* actRepeat = menu.addAction(tr("Add Repeat Mission"));

		// Disable "Add Repeat Mission" if one already exists
		const bool canAddRepeat = !hasRepeatBranch(idx);
		actRepeat->setEnabled(canAddRepeat);

		// Disable "Set as First Mission" if already first
		actSetFirst->setEnabled(!(idx == 0));

		QAction* chosen = menu.exec(e->globalPos());
		if (!chosen) {
			e->ignore();
			return;
		}

		if (chosen == actSetFirst) {
			Q_EMIT setFirstMissionRequested(idx);
			e->accept();
			return;
		}
		if (chosen == actDelete) {
			Q_EMIT deleteMissionRequested(idx);
			e->accept();
			return;
		}
		if (chosen == actRepeat) {
			Q_EMIT addRepeatBranchRequested(idx);
			e->accept();
			return;
		}
		e->ignore();
		return;
	}

	// END sink under cursor? no special menu
	if (m_endSink && m_endSink->sceneBoundingRect().contains(sp)) {
		QGraphicsView::contextMenuEvent(e);
		return;
	}

	// Empty space menu: "Add mission here"
	QMenu menu(this);
	QAction* actAdd = menu.addAction(tr("Add mission here"));

	// Snap to grid
	const qreal s = m_style.minorStep;
	const QPointF snapped(qRound(sp.x() / s) * s, qRound(sp.y() / s) * s);

	QAction* chosen = menu.exec(e->globalPos());
	if (chosen == actAdd) {
		Q_EMIT addMissionHereRequested(snapped);
		e->accept();
		return;
	}

	QGraphicsView::contextMenuEvent(e);
}

void CampaignMissionGraph::mouseMoveEvent(QMouseEvent* ev)
{
	if (m_drag.active && m_drag.preview) {
		const QPointF sp = mapToScene(ev->pos());
		m_drag.preview->setEndpoints(m_drag.srcPt, sp, /*sibIndex*/ 0, /*sibCount*/ 1);
		ev->accept();
		return;
	}
	QGraphicsView::mouseMoveEvent(ev);
}

void CampaignMissionGraph::drawGrid(QPainter* p, const QRectF& sceneRect) const
{
	const qreal minor = m_style.minorStep;
	const qreal major = minor * m_style.majorEvery;

	// Align to grid
	const qreal left = std::floor(sceneRect.left() / minor) * minor;
	const qreal right = std::ceil(sceneRect.right() / minor) * minor;
	const qreal top = std::floor(sceneRect.top() / minor) * minor;
	const qreal bottom = std::ceil(sceneRect.bottom() / minor) * minor;

	QPen penMinor(m_style.gridMinor);
	penMinor.setCosmetic(true);

	QPen penMajor(m_style.gridMajor);
	penMajor.setCosmetic(true);
	penMajor.setWidthF(1.2);

	// Vertical lines
	for (qreal x = left; x <= right; x += minor) {
		const bool isMajor = qFuzzyIsNull(std::fmod(std::abs(x), major));
		p->setPen(isMajor ? penMajor : penMinor);
		p->drawLine(QPointF(x, top), QPointF(x, bottom));
	}
	// Horizontal lines
	for (qreal y = top; y <= bottom; y += minor) {
		const bool isMajor = qFuzzyIsNull(std::fmod(std::abs(y), major));
		p->setPen(isMajor ? penMajor : penMinor);
		p->drawLine(QPointF(left, y), QPointF(right, y));
	}
}