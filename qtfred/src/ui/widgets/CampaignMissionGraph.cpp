#include "ui/widgets/campaignmissiongraph.h"

#include "mission/dialogs/CampaignEditorDialogModel.h"

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
	// filename (smaller)
	f.setPointSizeF(f.pointSizeF() - 1);
	p->setFont(f);
	const QRectF fileRect = QRectF(m_rect.left() + m_style.padding,
		m_rect.top() + m_style.padding,
		m_rect.width() - 2 * m_style.padding - m_style.badgeSize.width() - m_style.badgePad * 2,
		m_titleH);
	p->drawText(fileRect, Qt::AlignVCenter | Qt::AlignLeft | Qt::TextSingleLine, m_file);

	// mission name (bold)
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
	// Center right on the top edge; only outer half visible
	return mapToScene(QPointF(m_rect.center().x(), m_rect.top()));
}

QPointF MissionNodeItem::mainNubScenePos() const
{
	// Center right on the bottom edge; only outer half visible
	return mapToScene(QPointF(m_rect.center().x() - m_style.nubOffsetX, m_rect.bottom()));
}

QPointF MissionNodeItem::specialNubScenePos() const
{
	// Center right on the bottom edge; only outer half visible
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
	setAcceptHoverEvents(true);
	setFlag(QGraphicsItem::ItemIsSelectable, true);

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
	const QPointF p5 = dst; // final into inbound nub (vertical)

	QPainterPath path(p0);
	path.lineTo(p1);
	path.lineTo(p2);
	path.lineTo(p3);
	path.lineTo(p4);
	path.lineTo(p5);

	// Cache last segment for arrowhead (non-const method now)
	m_lastSegmentP1 = p4;
	m_lastSegmentP2 = p5;

	return path;
}

void EdgeItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	QGraphicsPathItem::paint(painter, option, widget);

	// Arrowhead at the end (toward m_lastSegmentP2)
	const QPointF a = m_lastSegmentP1;
	const QPointF b = m_lastSegmentP2;
	const QPointF v = b - a;
	const qreal len = std::hypot(v.x(), v.y());
	if (len < 0.001)
		return;

	const QPointF dir = v / len;
	const QPointF ortho(-dir.y(), dir.x());
	const qreal s = m_style.arrowSize;

	QPolygonF arrow;
	arrow << b << (b - dir * s + ortho * (s * 0.5)) << (b - dir * s - ortho * (s * 0.5));

	QBrush brush(this->pen().color());
	QPen pen = this->pen();
	pen.setCapStyle(Qt::SquareCap);
	painter->setPen(pen);
	painter->setBrush(brush);
	painter->drawPolygon(arrow);
}

void EdgeItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
	if (event->button() == Qt::LeftButton) {
		Q_EMIT edgeClicked(m_missionIndex, m_branchId);
		setSelected(true);
		setSelectedVisual(true);
		event->accept();
		return;
	}
	QGraphicsPathItem::mousePressEvent(event);
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

		// Labels (filename as primary for now)
		const QString fileLabel = QString::fromStdString(m.name);
		const QString nameLabel; // optional mission title later

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

	// Map mission name -> index (for branch targets)
	std::unordered_map<std::string, int> nameToIndex;
	nameToIndex.reserve(missions.size());
	for (int i = 0; i < (int)missions.size(); ++i) {
		nameToIndex[missions[i].name] = i;
	}

	// For each mission, build edges to other missions (skip END for now: empty next_mission_name)
	for (int i = 0; i < (int)missions.size(); ++i) {
		const auto& m = missions[i];
		const auto* srcNode = m_nodeItems[i];

		// Determine mode for special styling
		const auto mode = deriveMode(*m_model, i);

		// Prepare sibling counters for fan-out
		int mainTotal = 0, specTotal = 0;
		for (const auto& b : m.branches) {
			(b.is_loop || b.is_fork) ? ++specTotal : ++mainTotal;
		}
		int mainIdx = 0, specIdx = 0;

		for (const auto& b : m.branches) {
			// Skip END for now
			if (b.next_mission_name.empty())
				continue;

			auto it = nameToIndex.find(b.next_mission_name);
			if (it == nameToIndex.end()) {
				// Target mission name not found (possibly missing from list); skip gracefully
				continue;
			}
			const int j = it->second;
			const auto* dstNode = m_nodeItems[j];
			if (!srcNode || !dstNode)
				continue;

			const bool isSpecial = (b.is_loop || b.is_fork);
			const int sibCount = isSpecial ? specTotal : mainTotal;
			const int sibIndex = isSpecial ? specIdx++ : mainIdx++;

			const QPointF srcPt = isSpecial ? srcNode->specialNubScenePos() : srcNode->mainNubScenePos();
			const QPointF dstPt = dstNode->inboundNubScenePos();

			auto* edge = new EdgeItem(i, b.id, isSpecial, mode, m_style);
			edge->setEndpoints(srcPt, dstPt, sibIndex, sibCount);

			connect(edge, &EdgeItem::edgeClicked, this, [this](int mi, int bid) { Q_EMIT branchSelected(mi, bid); });

			m_scene->addItem(edge);
			m_edgeItems.push_back(edge);
		}
	}
}

// ----------------------------
// View helpers
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
	setFrameShape(QFrame::NoFrame); // optional
}

void CampaignMissionGraph::initScene()
{
	m_scene = new QGraphicsScene(this);
	setScene(m_scene);
	m_scene->setSceneRect(QRectF(-4000, -4000, 8000, 8000));
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
	m_scene->clear();
	m_nodeItems.clear();
	m_edgeItems.clear();

	if (!m_model)
		return;

	buildMissionNodes();
	buildMissionEdges();
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
	Q_UNUSED(rect);
	if (!m_gridVisible)
		return;

	p->save();
	p->setRenderHint(QPainter::Antialiasing, false);

	// Fill bg (in case view brush is transparent)
	p->fillRect(this->rect(), m_style.bgColor);

	drawGrid(p, mapToScene(this->rect()).boundingRect());

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

	// Vertical
	for (qreal x = left; x <= right; x += minor) {
		const bool isMajor = qFuzzyIsNull(std::fmod(std::abs(x), major));
		p->setPen(isMajor ? penMajor : penMinor);
		p->drawLine(QPointF(x, top), QPointF(x, bottom));
	}
	// Horizontal
	for (qreal y = top; y <= bottom; y += minor) {
		const bool isMajor = qFuzzyIsNull(std::fmod(std::abs(y), major));
		p->setPen(isMajor ? penMajor : penMinor);
		p->drawLine(QPointF(left, y), QPointF(right, y));
	}
}
