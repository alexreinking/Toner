#ifndef DATAREADER_H
#define DATAREADER_H

#include <QtCore>
typedef QVector<double> vec_type;

struct InstrumentModel {
    QString name;
    vec_type model;
    QVector<vec_type> originalTable;
    QVector<vec_type> filteredTable;
};

Q_DECLARE_METATYPE(InstrumentModel)

class DataReader
{
public:
    DataReader(const QString &fileName);
    ~DataReader();

    InstrumentModel getInstrumentModel();
    QVector<vec_type> getOriginalTable() { return origTable; }

private:
    vec_type getColumn(QVector<vec_type> table, int col);
    double getMean(vec_type col);
    double getMedian(vec_type col);
    double getQ1(vec_type col);
    double getQ3(vec_type col);

    bool isOutlier(double item, vec_type data);

    QFile *input;

    QVector<vec_type> origTable;
};

#endif // LOGREADER_H
