#include <QFont>
#include <QFontInfo>
#include <QFontMetrics>
#include "latexParser.h"
#include "particle.h"

constexpr const int Particle::lineWidth = 3;
constexpr const int Particle::vertexSize = 5;
constexpr const int Fermion::arrowSize = 10;
constexpr const int Boson::spacing = 10;
constexpr const int Higgs::dashLength = 10;
constexpr const int Hadron::margin = 10;

Particle::Particle(const QPoint &from, const QPoint &to): _from(from), _to(to){}
Particle::~Particle(){}

bool Particle::operator==(const Particle &other) const{
    return this->_from == other._from && this->_to == other._to;
}

QPoint Particle::startingPoint() const{
    return this->_from;
}

void Particle::setEndPoint(const QPoint &to){
    this->_to = to;
}

void Particle::setLabelText(const QString &text){
    this->_labelText = text;
}

QString Particle::labelText() const{
    return this->_labelText;
}

QDataStream &operator<<(QDataStream &dataStream, const Particle &particle){
    dataStream << particle._from << particle._to << particle._labelText;
    return dataStream;
}

QDataStream &operator>>(QDataStream &dataStream, Particle &particle){
    dataStream >> particle._from >> particle._to >> particle._labelText;
    return dataStream;
}

QVector2D Particle::direction() const{
    return QVector2D(this->_to - this->_from).normalized();
}

QVector2D Particle::normal() const{
    return QVector2D(this->direction().y(), -this->direction().x()).normalized();
}

void Particle::addLabel(QPainterPath *path, QString *svgCode) const{
    if(this->labelText().isEmpty()){
        return;
    }
    QFont defaultFont("Arial");
    QVector2D normal = this->normal();
    if(normal.x() < 0 && !dynamic_cast<const class Hadron*>(this)){
        normal = -normal;
    }
    QPoint anchorPoint = (this->_from + this->_to) / 2 + (normal * QFontInfo(defaultFont).pixelSize() * ((normal.y() > 0) ? 1 : 0.5)).toPoint();
    if(dynamic_cast<const class Vertex*>(this)){
        anchorPoint += QPoint(0, QFontInfo(defaultFont).pixelSize());
    }
    if(dynamic_cast<const class Hadron*>(this)){
        anchorPoint += (this->normal() * 15).toPoint();
    }
    if(normal.x() < 0){
        for(const Text &text: parseLatex(this->labelText(), anchorPoint, defaultFont, normal.x() == 0)){
            anchorPoint -= QPoint((normal.x() < 0) ? QFontMetrics(defaultFont).size(0, text.text).width() : 0, 0);
        }
    }
    for(const Text &text: parseLatex(this->labelText(), anchorPoint, defaultFont, normal.x() == 0)){
        if(path != nullptr){
            path->addText(text.position, text.font, text.text);
        }
        if(svgCode != nullptr){
            *svgCode += QString("<text x=\"%1\" y=\"%2\" style=\"font-size:%3pt;font-family:%4\">%5</text>").arg(text.position.x()).arg(text.position.y()).arg(QFontInfo(text.font).pointSizeF()).arg(QFontInfo(text.font).family(), text.text.toHtmlEscaped().replace(" ", "&#160;"))
                 .replace("??", "&#945;")
                 .replace("??", "&#946;")
                 .replace("??", "&#947;")
                 .replace("??", "&#915;")
                 .replace("??", "&#948;")
                 .replace("??", "&#916;")
                 .replace("??", "&#1013;")
                 .replace("??", "&#949;")
                 .replace("??", "&#950;")
                 .replace("??", "&#951;")
                 .replace("??", "&#952;")
                 .replace("??", "&#977;")
                 .replace("??", "&#920;")
                 .replace("??", "&#953;")
                 .replace("??", "&#954;")
                 .replace("??", "&#955;")
                 .replace("??", "&#923;")
                 .replace("??", "&#956;")
                 .replace("??", "&#957;")
                 .replace("??", "&#958;")
                 .replace("??", "&#926;")
                 .replace("??", "&#960;")
                 .replace("??", "&#928;")
                 .replace("??", "&#961;")
                 .replace("??", "&#1009;")
                 .replace("??", "&#963;")
                 .replace("??", "&#931;")
                 .replace("??", "&#964;")
                 .replace("??", "&#965;")
                 .replace("??", "&#978;")
                 .replace("??", "&#981;")
                 .replace("??", "&#966;")
                 .replace("??", "&#934;")
                 .replace("??", "&#967;")
                 .replace("??", "&#968;")
                 .replace("??", "&#936;")
                 .replace("??", "&#969;")
                 .replace("??", "&#937;")
                 .replace("??", "&#177;")
                 .replace("??", "&#773;");
        }
    }
}

const QList<QPoint> Fermion::arrowPoints() const{
    const QPoint arrowBack = (this->_from + this->_to) / 2 - (this->direction() * arrowSize / 2).toPoint();
    const QPoint arrowFront = (this->_from + this->_to) / 2 + (this->direction() * arrowSize / 2).toPoint();
    return QList<QPoint>({arrowBack + (this->normal() * arrowSize / 2).toPoint(), arrowFront, arrowBack - (this->normal() * arrowSize / 2).toPoint()});
}

QString Fermion::svgCode() const{
    QString toReturn = QString("<line x1=\"%1\" y1=\"%2\" x2=\"%3\" y2=\"%4\" fill=\"none\" stroke=\"black\" stroke-width=\"2\"/>").arg(this->_from.x()).arg(this->_from.y()).arg(this->_to.x()).arg(this->_to.y());
    toReturn += "<polygon fill=\"black\" stroke=\"none\" points=\"";
    for(const QPoint &point: this->arrowPoints()){
        toReturn += QString("%1,%2 ").arg(point.x()).arg(point.y());
    }
    toReturn += "\"/>";
    this->addLabel(nullptr, &toReturn);
    return toReturn;
}

QPainterPath Fermion::painterPath() const{
    QPainterPathStroker stroker;
    stroker.setWidth(lineWidth);
    QPainterPath lines;
    lines.moveTo(this->_from);
    lines.lineTo(this->_to);

    QPainterPath path;
    path.setFillRule(Qt::WindingFill);
    path.addPath(stroker.createStroke(lines));
    const QList<QPoint> &arrowPoints = this->arrowPoints();
    path.moveTo(arrowPoints[0]);
    path.lineTo(arrowPoints[1]);
    path.lineTo(arrowPoints[2]);
    path.closeSubpath();
    this->addLabel(&path, nullptr);
    return path;
}

const QList<QPoint> Boson::points() const{
    const int length = QVector2D(this->_to - this->_from).length();
    QList<QPoint> toReturn;
    for(int i = spacing / 2; i < length; i += spacing){
        toReturn.append(this->_from + (i * this->direction() + ((((i / spacing) % 2) ? this->normal() : -this->normal())) * spacing).toPoint());
    }
    toReturn.append(this->_to);
    return toReturn;
}

QString WeakBoson::svgCode() const{
    QString toReturn = QString("<path fill=\"none\" stroke=\"black\" stroke-width=\"2\" d=\"M%1 %2").arg(this->_from.x()).arg(this->_from.y());
    for(const QPoint &point: this->points()){
        toReturn += QString("L%1 %2").arg(point.x()).arg(point.y());
    }
    toReturn += "\"/>";
    this->addLabel(nullptr, &toReturn);
    return toReturn;
}

QPainterPath WeakBoson::painterPath() const{
    QPainterPathStroker stroker;
    stroker.setWidth(lineWidth);
    QPainterPath lines;
    lines.moveTo(this->_from);
    for(const QPoint &point: this->points()){
        lines.lineTo(point);
    }

    QPainterPath path;
    path.setFillRule(Qt::WindingFill);
    path.addPath(stroker.createStroke(lines));
    this->addLabel(&path, nullptr);
    return path;
}

void Photon::iterateOverPoints(const std::function<void(const QPoint&, const QPoint&, const QPoint&)> &callback) const{
    const QPoint displacement = (this->direction() * spacing / 2).toPoint();
    QPoint previousPoint = this->_from - displacement;
    for(const QPoint &point: this->points()){
        callback(previousPoint + displacement, (point == this->_to ? point : point - displacement), point);
        previousPoint = point;
    }
}

void Gluon::iterateOverPoints(const std::function<void(const QPoint&, const QPoint&, const QPoint&)> &callback) const{
    const QPoint displacement = (this->direction() * spacing / 2).toPoint();
    const QList<QPoint> &points = this->points();
    for(int i = 0; i < points.size(); i++){
        const QPoint &point = points[i];
        const QPoint &previousPoint = i ? points[i - 1] : (this->_from - displacement);
        if(i == 0){
            callback(previousPoint + displacement, (point == this->_to ? point : point - displacement), point);
        }
        else if(i % 2){
            callback(previousPoint + displacement * 2, (point == this->_to ? point : point + displacement * 2), point);
        }
        else{
            callback(previousPoint - displacement * 2, (point == this->_to ? point : point - displacement * 2), point);
        }
    }
}

QString MasslessBoson::svgCode() const{
    QString toReturn = QString("<path fill=\"none\" stroke=\"black\" stroke-width=\"2\" d=\"M%1 %2").arg(this->_from.x()).arg(this->_from.y());
    this->iterateOverPoints([&toReturn](const QPoint &c1, const QPoint &c2, const QPoint &end){
        toReturn += QString("C%1 %2,%3 %4,%5 %6").arg(c1.x()).arg(c1.y()).arg(c2.x()).arg(c2.y()).arg(end.x()).arg(end.y());
    });
    toReturn += "\"/>";
    this->addLabel(nullptr, &toReturn);
    return toReturn;
}

QPainterPath MasslessBoson::painterPath() const{
    QPainterPathStroker stroker;
    stroker.setWidth(lineWidth);
    QPainterPath lines;
    lines.moveTo(this->_from);
    this->iterateOverPoints([&lines](const QPoint &c1, const QPoint &c2, const QPoint &end){
        lines.cubicTo(c1, c2, end);
    });

    QPainterPath path;
    path.setFillRule(Qt::WindingFill);
    path.addPath(stroker.createStroke(lines));
    this->addLabel(&path, nullptr);
    return path;
}

QString Higgs::svgCode() const{
    QString toReturn = QString("<line x1=\"%1\" y1=\"%2\" x2=\"%3\" y2=\"%4\" fill=\"none\" stroke=\"black\" stroke-width=\"2\" stroke-dasharray=\"%5\"/>").arg(this->_from.x()).arg(this->_from.y()).arg(this->_to.x()).arg(this->_to.y()).arg(dashLength);
    this->addLabel(nullptr, &toReturn);
    return toReturn;
}

QPainterPath Higgs::painterPath() const{
    QPainterPathStroker stroker;
    stroker.setWidth(lineWidth);
    QPainterPath lines;
    const int length = QVector2D(this->_to - this->_from).length();
    for(int i = 0; i < length; i += dashLength * 2){
        lines.moveTo(this->_from + (i * this->direction()).toPoint());
        if(i + dashLength > length){
            lines.lineTo(this->_to);
        }
        else{
            lines.lineTo(this->_from + ((i + dashLength) * this->direction()).toPoint());
        }
    }

    QPainterPath path;
    path.setFillRule(Qt::WindingFill);
    path.addPath(stroker.createStroke(lines));
    this->addLabel(&path, nullptr);
    return path;
}

QString GenericBoson::svgCode() const{
    QString toReturn = QString("<line x1=\"%1\" y1=\"%2\" x2=\"%3\" y2=\"%4\" fill=\"none\" stroke=\"black\" stroke-width=\"2\"/>").arg(this->_from.x()).arg(this->_from.y()).arg(this->_to.x()).arg(this->_to.y());
    this->addLabel(nullptr, &toReturn);
    return toReturn;
}

QPainterPath GenericBoson::painterPath() const{
    QPainterPathStroker stroker;
    stroker.setWidth(lineWidth);
    QPainterPath line;
    line.moveTo(this->_from);
    line.lineTo(this->_to);

    QPainterPath path;
    path.setFillRule(Qt::WindingFill);
    path.addPath(stroker.createStroke(line));
    this->addLabel(&path, nullptr);
    return path;
}

QString Hadron::svgCode() const{
    QString toReturn = QString("<path d=\"M%1 %2L%3 %4L%5 %6L%7 %8\" fill=\"none\" stroke=\"black\" stroke-width=\"2\"/>").arg(this->_from.x() + (-this->direction() * margin).x()).arg(this->_from.y() + (-this->direction() * margin).y()).arg(this->_from.x() + (this->normal() * margin - this->direction() * margin).x()).arg(this->_from.y() + (this->normal() * margin - this->direction() * margin).y()).arg(this->_to.x() + (this->normal() * margin + this->direction() * margin).x()).arg(this->_to.y() + (this->normal() * margin + this->direction() * margin).y()).arg(this->_to.x() + (this->direction() * margin).x()).arg(this->_to.y() + (this->direction() * margin).y());
    this->addLabel(nullptr, &toReturn);
    return toReturn;
}

QPainterPath Hadron::painterPath() const{
    QPainterPathStroker stroker;
    stroker.setWidth(lineWidth);
    QPainterPath line;
    line.moveTo(this->_from + (-this->direction() * margin).toPoint());
    line.lineTo(this->_from + (this->normal() * margin - this->direction() * margin).toPoint());
    line.lineTo(this->_to + (this->normal() * margin + this->direction() * margin).toPoint());
    line.lineTo(this->_to + (this->direction() * margin).toPoint());

    QPainterPath path;
    path.setFillRule(Qt::WindingFill);
    path.addPath(stroker.createStroke(line));
    this->addLabel(&path, nullptr);
    return path;
}

Vertex::Vertex(const QPoint &point): Particle(point, point){}

QString Vertex::svgCode() const{
    QString toReturn;
    this->addLabel(nullptr, &toReturn);
    return toReturn;
}

QPainterPath Vertex::painterPath() const{
    QPainterPath path;
    path.setFillRule(Qt::WindingFill);
    path.addEllipse(this->_from, vertexSize, vertexSize);
    this->addLabel(&path, nullptr);
    return path;
}
