#include "ui/widgets/campaignmissiongraph.h"

#include "mission/dialogs/CampaignEditorDialogModel.h"

#include "mission/missionparse.h"

#include <QScrollBar>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QWheelEvent>
#include <QtMath>
#include <cmath>
#include <unordered_map>

using detail::EdgeItem;
using detail::MissionNodeItem;
using detail::SpecialMode;
using fso::fred::dialogs::CampaignEditorDialogModel;

// ----------------------------
// MissionNodeItem definitions
// ----------------------------

MissionNodeItem::MissionNodeItem(int missionIndex,
	const QString& fileLabel,
	const QString& nameLabel,
	int graphColorRgb,
	SpecialMode mode,
	int mainBranchCount,
	int specialBranchCount,
	const CampaignGraphStyle& style,
	QGraphicsItem* parent)
	: QGraphicsObject(parent), m_idx(missionIndex), m_file(fileLabel), m_name(nameLabel), m_graphColor(graphColorRgb),
	  m_mode(mode), m_mainCount(mainBranchCount), m_specCount(specialBranchCount), m_style(style)
{
	setFlags(ItemIsSelectable | ItemSendsScenePositionChanges);
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

	const qreal topNubY = m_rect.top();       // center exactly on edge -> half visible
	const qreal bottomNubY = m_rect.bottom(); // center exactly on edge -> half visible
	const qreal centerX = m_rect.center().x();
	const QPointF inboundNub(centerX, topNubY);
	const QPointF mainNub(centerX - m_style.nubOffsetX, bottomNubY);
	const QPointF specNub(centerX + m_style.nubOffsetX, bottomNubY);

	// --- 1) NUBS FIRST (under the card so only half shows) ---
	p->setPen(borderPen);

	// Top inbound (green)
	p->setBrush(m_style.inboundGreen);
	p->drawEllipse(inboundNub, m_style.nubRadius, m_style.nubRadius);

	// Bottom main (blue)
	p->setBrush(m_style.mainBlue);
	p->drawEllipse(mainNub, m_style.nubRadius, m_style.nubRadius);

	// Bottom special (loop/fork color)
	const QColor specColor = (m_mode == SpecialMode::Loop) ? m_style.loopOrange : m_style.forkPurple;
	p->setBrush(specColor);
	p->drawEllipse(specNub, m_style.nubRadius, m_style.nubRadius);

	// --- 2) CARD ON TOP (covers inner halves of the nubs) ---
	p->setPen(borderPen);
	p->setBrush(m_style.nodeFill);
	p->drawPath(roundedRectPath(m_rect, m_style.nodeRadius));

	// 3) User color stripe (on top of card)
	if (m_graphColor >= 0) {
		const QColor stripe((m_graphColor >> 16) & 0xFF, (m_graphColor >> 8) & 0xFF, (m_graphColor) & 0xFF);
		p->fillRect(
			QRectF(m_rect.left(), m_rect.top() + m_style.padding + m_titleH, m_rect.width(), m_style.stripeHeight),
			stripe);
	}

	// 4) Badge (icon pill) in header; disabled if special branches exist
	const bool badgeDisabled = (m_specCount > 0);
	const QRectF badgeRect(m_rect.right() - m_style.badgePad - m_style.badgeSize.width(),
		m_rect.top() + m_style.badgePad,
		m_style.badgeSize.width(),
		m_style.badgeSize.height());
	m_badgeRect = badgeRect;

	QColor badgeCol = (m_mode == SpecialMode::Loop) ? m_style.loopOrange : m_style.forkPurple;
	if (badgeDisabled)
		badgeCol = m_style.badgeDisabled;

	p->setBrush(badgeCol);
	p->setPen(Qt::NoPen);
	p->drawRoundedRect(badgeRect, badgeRect.height() / 2.0, badgeRect.height() / 2.0);

	// Icon placeholder (swap for your QIcon later)
	p->setPen(QPen(Qt::white, 2));
	if (m_mode == SpecialMode::Loop) {
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

	// 5) Text: filename (top) and mission name (below stripe)
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
			Q_EMIT specialModeToggleRequested(m_idx);
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

void MissionNodeItem::updateGeometry()
{
	m_titleH = 18.0;
	m_nameH = 24.0;
	m_rect = QRectF(0, 0, 240.0, 120.0); // sync with CampaignGraphStyle defaults
}

QPointF MissionNodeItem::inboundNubScenePos() const
{
	return mapToScene(QPointF(m_rect.center().x(), m_rect.top()));
}

QPointF MissionNodeItem::mainNubScenePos() const
{
	return mapToScene(QPointF(m_rect.center().x() - m_style.nubOffsetX, m_rect.bottom()));
}

QPointF MissionNodeItem::specialNubScenePos() const
{
	return mapToScene(QPointF(m_rect.center().x() + m_style.nubOffsetX, m_rect.bottom()));
}

// ----------------------------
// EdgeItem definitions
// ----------------------------

EdgeItem::EdgeItem(int missionIndex,
	int branchId,
	bool isSpecial,
	SpecialMode mode,
	const CampaignGraphStyle& style,
	QGraphicsItem* parent)
	: QObject(), QGraphicsPathItem(parent), m_missionIndex(missionIndex), m_branchId(branchId), m_isSpecial(isSpecial),
	  m_mode(mode), m_style(style)
{
	setZValue(5); // below nodes (nodes use 10)
	// non-interactive
	setAcceptedMouseButtons(Qt::NoButton);
	setAcceptHoverEvents(false);

	// color / dash
	if (!m_isSpecial) {
		m_color = m_style.mainBlue;
		m_dash = Qt::SolidLine;
	} else {
		if (m_mode == SpecialMode::Loop) {
			m_color = m_style.loopOrange;
			m_dash = Qt::DashLine;
		} else {
			m_color = m_style.forkPurple;
			m_dash = Qt::DashDotLine;
		}
	}
	QPen pen(m_color, m_style.edgeWidth, m_dash, Qt::RoundCap, Qt::RoundJoin);
	pen.setCosmetic(true);
	setPen(pen);
}

void EdgeItem::setSelectedVisual(bool sel)
{
	// kept for future; no-op visual tweak if desired
	QPen pen = this->pen();
	pen.setWidthF(sel ? m_style.edgeWidth + 1.5 : m_style.edgeWidth);
	setPen(pen);
	update();
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
	// Sibling offset centered around zero
	const qreal offset = (siblingCount > 1) ? ((siblingIndex - (siblingCount - 1) * 0.5) * m_style.fanoutStep) : 0.0;

	// Start with a short vertical out from source
	const QPointF p0 = src;
	const QPointF p1 = p0 + QPointF(0, m_style.fanoutStart); // down

	// Fan-out horizontally from source side
	const bool sourceIsRight = (m_isSpecial); // special nub is on right side of node
	const qreal fanDir = sourceIsRight ? +1.0 : -1.0;
	const QPointF p2 = p1 + QPointF(offset * fanDir, 0);

	// Approach target: vertical near target then into it
	const QPointF p3(dst.x(), p2.y());
	const QPointF p4(dst.x(), dst.y() - m_style.fanoutStart);

	// Stop short of the target nub so the head stays visible
	const qreal inset = std::max<qreal>(2.0, m_style.arrowTargetInset);
	const QPointF p5 = dst - QPointF(0, inset);

	QPainterPath path(p0);
	path.lineTo(p1);
	path.lineTo(p2);
	path.lineTo(p3);
	path.lineTo(p4);
	path.lineTo(p5);

	// Cache for painting
	m_lastSegmentP1 = p4;
	m_lastSegmentP2 = p5;

	// Full point list for arrow placement
	m_points = {p0, p1, p2, p3, p4, p5};

	return path;
}

QPainterPath EdgeItem::buildSelfLoopPath(const QRectF& node, bool sourceIsRightSide, int siblingIndex, int siblingCount)
{
	// Offset spread for multiple self-loops
	const qreal spread =
		(siblingCount > 1) ? ((siblingIndex - (siblingCount - 1) * 0.5) * m_style.selfLoopSpread) : 0.0;

	const qreal sideX = sourceIsRightSide ? (node.right() + m_style.selfLoopMargin + std::abs(spread))
										  : (node.left() - m_style.selfLoopMargin - std::abs(spread));

	const QPointF src = sourceIsRightSide ? QPointF(node.center().x() + m_style.nubOffsetX, node.bottom())
										  : QPointF(node.center().x() - m_style.nubOffsetX, node.bottom());

	const QPointF dst(node.center().x(), node.top());

	// Rectangular wrap outside node bounds
	const QPointF p0 = src;
	const QPointF p1 = p0 + QPointF(0, m_style.fanoutStart);
	const QPointF p2(sideX, p1.y());
	const QPointF p3(sideX, node.top() - m_style.fanoutStart);

	// Stop short of inbound nub so arrow is visible (not under node)
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

	// Helper to draw a single arrow at 'tip' pointing along 'dir' (dir must be normalized)
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

	// 1) Arrow at the very end (towards m_lastSegmentP2)
	{
		const QPointF a = m_lastSegmentP1;
		const QPointF b = m_lastSegmentP2;
		const QPointF v = b - a;
		const qreal len = std::hypot(v.x(), v.y());
		if (len > 0.001) {
			drawArrowAt(b, v / len, m_style.arrowSize);
		}
	}

	// 2) Arrows at regular intervals along each straight segment (no turn arrows)
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

		const qreal start = std::max(interval, endMargin); // first arrow along the segment
		for (qreal d = start; d <= segLen - endMargin; d += interval) {
			const QPointF pos = p1 + dir * d;
			drawArrowAt(pos, dir, m_style.arrowSize);
		}
	}
}

// ----------------------------
// CampaignMissionGraph impl
// ----------------------------

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

	// Ensure when the scene is smaller than the viewport it sits at the top-left,
	// and when we reset the scene rect we can scroll to (0,0).
	setAlignment(Qt::AlignLeft | Qt::AlignTop);
}

void CampaignMissionGraph::initScene()
{
	m_scene = new QGraphicsScene(this);
	setScene(m_scene);
}

void CampaignMissionGraph::updateSceneRectToContent(bool scrollToTopLeft)
{
	if (!m_scene)
		return;

	QRectF itemsRect = m_scene->itemsBoundingRect();

	// If no items yet, create a small default rect near (0,0)
	if (!itemsRect.isValid() || itemsRect.isEmpty()) {
		itemsRect = QRectF(0, 0, m_style.nodeSize.width() * 3.0, m_style.nodeSize.height() * 2.0);
	}

	// Expand by margins so there’s breathing room to pan/drag
	const QRectF sceneRect = itemsRect.adjusted(-m_style.contentMarginX,
		-m_style.contentMarginY,
		+m_style.contentMarginX,
		+m_style.contentMarginY);
	m_scene->setSceneRect(sceneRect);

	if (scrollToTopLeft) {
		// Align top-left and reset scroll bars to the minimum (top-left corner)
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
	rebuildAll();
}

void CampaignMissionGraph::rebuildAll()
{
	if (!m_scene)
		return;

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
		updateSceneRectToContent(true);
		return;
	}

	buildMissionNodes();
	buildMissionEdges();

	// Size scene to the items and put the view at the top-left
	updateSceneRectToContent(true);
}

static detail::SpecialMode deriveMode(const CampaignEditorDialogModel& model, int missionIdx)
{
	const auto& missions = model.getCampaignMissions();
	if (missionIdx < 0 || missionIdx >= (int)missions.size())
		return detail::SpecialMode::Loop;
	const auto& m = missions[missionIdx];
	bool anyLoop = false, anyFork = false;
	for (const auto& b : m.branches) {
		anyLoop |= b.is_loop;
		anyFork |= b.is_fork;
	}
	if (anyLoop)
		return detail::SpecialMode::Loop;
	if (anyFork)
		return detail::SpecialMode::Fork;
	return detail::SpecialMode::Loop; // default when none exist
}

void CampaignMissionGraph::buildMissionNodes()
{
	const auto& missions = m_model->getCampaignMissions();
	const QPointF fallbackStep(240, 160);

	m_nodeItems.reserve(missions.size());

	for (int i = 0; i < (int)missions.size(); ++i) {
		const auto& m = missions[i];

		// Position
		QPointF pos;
		if (m.graph_x != INT_MIN && m.graph_y != INT_MIN) {
			pos = QPointF(m.graph_x, m.graph_y);
		} else {
			pos = QPointF(m.position * fallbackStep.x(), m.level * fallbackStep.y());
		}

		// Branch counts & special mode
		int mainCount = 0, specCount = 0;
		for (const auto& b : m.branches) {
			(b.is_loop || b.is_fork) ? ++specCount : ++mainCount;
		}
		const auto mode = deriveMode(*m_model, i);

		// Labels
		QString fileLabel = QString::fromStdString(m.filename);
		QString nameLabel;

		mission mission_info;
		if (get_mission_info(m.filename.c_str(), &mission_info) == 0) {
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

	// Map mission name -> index (for branch targets)
	std::unordered_map<std::string, int> nameToIndex;
	nameToIndex.reserve(missions.size());
	for (int i = 0; i < (int)missions.size(); ++i) {
		nameToIndex[missions[i].filename] = i;
	}

	// For each mission, build edges to other missions (and self)
	for (int i = 0; i < (int)missions.size(); ++i) {
		const auto& m = missions[i];
		const auto* srcNode = m_nodeItems[i];

		const auto mode = deriveMode(*m_model, i);

		// Prepare sibling counters for fan-out
		int mainTotal = 0, specTotal = 0;
		for (const auto& b : m.branches) {
			(b.is_loop || b.is_fork) ? ++specTotal : ++mainTotal;
		}
		int mainIdx = 0, specIdx = 0;

		for (const auto& b : m.branches) {
			// MAIN ? END (empty target) allowed
			if (!b.is_loop && !b.is_fork && b.next_mission_name.empty()) {
				if (m_endSink) {
					const int sibCount = mainTotal;
					const int sibIndex = mainIdx++;

					const QPointF srcPt = srcNode->mainNubScenePos();
					const QPointF dstPt = m_endSink->inboundAnchorScenePos();

					auto* edge = new detail::EdgeItem(i, b.id, /*isSpecial*/ false, mode, m_style);
					edge->setEndpoints(srcPt, dstPt, sibIndex, sibCount);
					m_scene->addItem(edge);
					m_edgeItems.push_back(edge);
				}
				continue; // handled
			}

			// SPECIAL with empty target is nonsensical; ignore
			if ((b.is_loop || b.is_fork) && b.next_mission_name.empty()) {
				// optional: qWarning() << "Special branch with empty next_mission_name ignored for mission" << i;
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

			if (i == j) {
				// Self-loop: draw outside node (unless disabled)
				if (m_style.showSelfLoops) {
					const QRectF nodeRectScene = srcNode->mapRectToScene(srcNode->boundingRect());
					const bool sourceRight = isSpecial; // special nub on right; main on left
					edge->setSelfLoop(nodeRectScene, sourceRight, sibIndex, sibCount);
					m_scene->addItem(edge);
					m_edgeItems.push_back(edge);
				} else {
					delete edge;
				}
				continue;
			}

			// Normal edge
			const QPointF srcPt = isSpecial ? srcNode->specialNubScenePos() : srcNode->mainNubScenePos();
			const QPointF dstPt = dstNode->inboundNubScenePos();

			edge->setEndpoints(srcPt, dstPt, sibIndex, sibCount);

			m_scene->addItem(edge);
			m_edgeItems.push_back(edge);
		}
	}
}

void CampaignMissionGraph::ensureEndSink()
{
	// Needs END only if any MAIN branch (non-special) ends the campaign
	bool needsEnd = false;
	if (m_model) {
		const auto& missions = m_model->getCampaignMissions();
		for (const auto& m : missions) {
			for (const auto& b : m.branches) {
				if (!b.is_loop && !b.is_fork && b.next_mission_name.empty()) { // MAIN ? END only
					needsEnd = true;
					break;
				}
			}
			if (needsEnd)
				break;
		}
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
		m_endSink->setZValue(9.0); // above edges (5), below nodes (10)
		m_scene->addItem(m_endSink);
	}

	// Position below the union of mission nodes
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

	// Fill the currently exposed SCENE area (not the widget rect)
	p->fillRect(rect, m_style.bgColor);

	// Draw grid over the same scene rect
	drawGrid(p, rect);

	p->restore();
}

void CampaignMissionGraph::drawGrid(QPainter* p, const QRectF& sceneRect)
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