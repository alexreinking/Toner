#include "datareader.h"

DataReader::DataReader(const QString &fileName)
{
    input = new QFile(fileName);
    origTable = QVector<vec_type>();
}

DataReader::~DataReader()
{
    input->close();
    delete input;
}

InstrumentModel DataReader::getInstrumentModel()
{
    InstrumentModel ret;
    QVector<vec_type> table;
    if(input->open(QIODevice::ReadOnly)) {
        QTextStream stream(input);
        QString line;
        do { //Build table!
            line = stream.readLine();
            QStringList tabs = line.split('\t');
            if(tabs.size() != 21)
                continue;
            vec_type row;
            if(!ret.name.size())
                ret.name = tabs.at(0).simplified();
            row << tabs.at(2).toDouble()  << tabs.at(7).toDouble()
                << tabs.at(8).toDouble()  << tabs.at(10).toDouble()
                << tabs.at(12).toDouble() << tabs.at(13).toDouble()
                << tabs.at(15).toDouble() << tabs.at(17).toDouble()
                << tabs.at(18).toDouble() << tabs.at(20).toDouble();
            table.append(row);
        } while(!line.isNull());
        ret.originalTable = table;
        origTable = table;
        vec_type col1 = getColumn(table,0);
        for(int i = 0; i < table.size(); i++) {
            if(isOutlier(table.at(i).at(0),col1))
                table.remove(i--);
        }
        vec_type col2 = getColumn(table,1);
        for(int i = 0; i < table.size(); i++) {
            if(isOutlier(table.at(i).at(1),col2))
                table.remove(i--);
        }
        vec_type col3 = getColumn(table,4);
        for(int i = 0; i < table.size(); i++) {
            if(isOutlier(table.at(i).at(4),col3))
                table.remove(i--);
        }
        vec_type col4 = getColumn(table,7);
        for(int i = 0; i < table.size(); i++) {
            if(isOutlier(table.at(i).at(7),col4))
                table.remove(i--);
        }
        vec_type result;
        result << getMean(getColumn(table,2))
               << getMean(getColumn(table,5))
               << getMean(getColumn(table,8))
               << getMean(getColumn(table,3))
               << getMean(getColumn(table,6))
               << getMean(getColumn(table,9));
        ret.model = result;
        ret.filteredTable = table;
        return ret;
    }
    ret.name = "Error";
    ret.model = vec_type();
    return ret;
}

bool DataReader::isOutlier(double item, vec_type data)
{
    qSort(data);
    double q1 = getQ1(data);
    double q3 = getQ3(data);
    return (item > (q3+1.5*(q3-q1)) || item < (q1-1.5*(q3-q1)));
}

double DataReader::getQ1(vec_type col) {
    if(col.size() % 2 == 0)
        return (col[col.size()/4] + col[col.size()/4-1])/2.0;
    return col[col.size()/4-1];
}

double DataReader::getQ3(vec_type col)
{
    if(col.size() % 2 == 0)
        return (col[3*col.size()/4] + col[3*col.size()/4-1])/2.0;
    return col[col.size()/2+col.size()/4];
}

double DataReader::getMedian(vec_type col)
{
    if(col.size() % 2 == 0)
        return (col[col.size()/4]+col[col.size()/4-1])/2.0;
    return col[col.size()];
}

double DataReader::getMean(vec_type col)
{
    double sum = 0.0;
    foreach(double num, col) {
        sum += num;
    }
    sum /= col.size();
    return sum;
}

vec_type DataReader::getColumn(QVector<vec_type> table, int col)
{
    vec_type ret;
    foreach(vec_type row, table) {
        ret << row.at(col);
    }
    return ret;
}
