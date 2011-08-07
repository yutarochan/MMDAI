#ifndef MOTIONBASEMODEL_H
#define MOTIONBASEMODEL_H

#include <QtGui/QAbstractItemView>
#include <vpvl/PMDModel.h>

namespace vpvl {
class VMDMotion;
class VPDPose;
}

class MotionBaseModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit MotionBaseModel(QObject *parent = 0);
    ~MotionBaseModel();

    virtual bool loadMotion(vpvl::VMDMotion *motion, vpvl::PMDModel *model) = 0;
    virtual void saveMotion(vpvl::VMDMotion *motion) = 0;
    void saveState();
    void restoreState();
    void discardState();

    vpvl::PMDModel *selectedModel() const { return m_model; }

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

public slots:
    virtual void setPMDModel(vpvl::PMDModel *model) = 0;

signals:
    void modelDidChange(vpvl::PMDModel *model);

protected:
    bool updateModel();

    vpvl::PMDModel *m_model;
    vpvl::PMDModel::State *m_state;
    QList<QString> m_keys;
    QHash< QPair<int, int>, QVariant > m_values;
};

#endif // MOTIONBASEMODEL_H
