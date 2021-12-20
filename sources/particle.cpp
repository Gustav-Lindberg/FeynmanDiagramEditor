#include <QFont>
#include <QFontInfo>
#include "latexParser.h"
#include "particle.h"

const int Fermion::arrowSize = 10;
const int Boson::spacing = 10;
const int Higgs::dashLength = 10;

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
    if(normal.x() < 0){
        normal = -normal;
    }
    const QPoint anchorPoint = (this->_from + this->_to) / 2 + (normal * QFontInfo(defaultFont).pixelSize() * ((normal.y() > 0) ? 1 : 0.5)).toPoint();
    for(const Text &text: parseLatex(this->labelText(), anchorPoint, defaultFont, normal.x() == 0)){
        if(path != nullptr){
            path->addText(text.position, text.font, text.text);
        }
        if(svgCode != nullptr){
            *svgCode += QString("<text x=\"%1\" y=\"%2\" style=\"font-size:%3pt;font-family:%4\">%5</text>").arg(text.position.x()).arg(text.position.y()).arg(QFontInfo(text.font).pointSizeF()).arg(QFontInfo(text.font).family(), text.text.toHtmlEscaped())
                 .replace("α", "&#945;")
                 .replace("β", "&#946;")
                 .replace("γ", "&#947;")
                 .replace("Γ", "&#915;")
                 .replace("δ", "&#948;")
                 .replace("Δ", "&#916;")
                 .replace("ϵ", "&#1013;")
                 .replace("ε", "&#949;")
                 .replace("ζ", "&#950;")
                 .replace("η", "&#951;")
                 .replace("θ", "&#952;")
                 .replace("ϑ", "&#977;")
                 .replace("Θ", "&#920;")
                 .replace("ι", "&#953;")
                 .replace("κ", "&#954;")
                 .replace("λ", "&#955;")
                 .replace("Λ", "&#923;")
                 .replace("µ", "&#956;")
                 .replace("ν", "&#957;")
                 .replace("ξ", "&#958;")
                 .replace("Ξ", "&#926;")
                 .replace("π", "&#960;")
                 .replace("Π", "&#928;")
                 .replace("ρ", "&#961;")
                 .replace("ϱ", "&#1009;")
                 .replace("σ", "&#963;")
                 .replace("Σ", "&#931;")
                 .replace("τ", "&#964;")
                 .replace("υ", "&#965;")
                 .replace("ϒ", "&#978;")
                 .replace("ϕ", "&#981;")
                 .replace("φ", "&#966;")
                 .replace("Φ", "&#934;")
                 .replace("χ", "&#967;")
                 .replace("ψ", "&#968;")
                 .replace("Ψ", "&#936;")
                 .replace("ω", "&#969;")
                 .replace("Ω", "&#937;");
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
    QPainterPath path;
    path.moveTo(this->_from);
    path.lineTo(this->_to);
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
    QPainterPath path;
    path.moveTo(this->_from);
    for(const QPoint &point: this->points()){
        path.lineTo(point);
    }
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
    QPainterPath path;
    path.moveTo(this->_from);
    this->iterateOverPoints([&path](const QPoint &c1, const QPoint &c2, const QPoint &end){
        path.cubicTo(c1, c2, end);
    });
    this->addLabel(&path, nullptr);
    return path;
}

QString Higgs::svgCode() const{
    QString toReturn = QString("<line x1=\"%1\" y1=\"%2\" x2=\"%3\" y2=\"%4\" fill=\"none\" stroke=\"black\" stroke-width=\"2\" stroke-dasharray=\"%5\"/>").arg(this->_from.x()).arg(this->_from.y()).arg(this->_to.x()).arg(this->_to.y()).arg(dashLength);
    this->addLabel(nullptr, &toReturn);
    return toReturn;
}

QPainterPath Higgs::painterPath() const{
    QPainterPath path;
    const int length = QVector2D(this->_to - this->_from).length();
    for(int i = 0; i < length; i += dashLength * 2){
        path.moveTo(this->_from + (i * this->direction()).toPoint());
        if(i + dashLength > length){
            path.lineTo(this->_to);
        }
        else{
            path.lineTo(this->_from + ((i + dashLength) * this->direction()).toPoint());
        }
    }
    this->addLabel(&path, nullptr);
    return path;
}
