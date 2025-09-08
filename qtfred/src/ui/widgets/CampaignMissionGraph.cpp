#include "ui/widgets/campaignmissiongraph.h"

#include "mission/dialogs/CampaignEditorDialogModel.h"

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QWheelEvent>
#include <QtMath>

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
		p->drawLine(QPointF(c.x(), badgeRect.top() + 3), QPointF(c.x() - 6, badgeRect.top() + 8));
		p->drawLine(QPointF(c.x(), badgeRect.top() + 3), QPointF(c.x() + 6, badgeRect.top() + 8));
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
	if (!m_model)
		return;

	buildMissionNodes();
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