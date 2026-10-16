/*
  DMX Total Control
  zonemanager.h

  Copyright (c) DMX Total Control contributors

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/

#ifndef ZONEMANAGER_H
#define ZONEMANAGER_H

#include <QAbstractListModel>
#include <QList>
#include <QSet>

class QXmlStreamReader;
class QXmlStreamWriter;
class Zone;
class Doc;

#define KXMLDTCZones       QStringLiteral("Zones")
#define KXMLDTCZonesNextID QStringLiteral("NextID")

/**
 * The list of zones of the current show
 */
class ZoneManager : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(bool paused READ paused WRITE setPaused NOTIFY pausedChanged)

public:
    enum Roles
    {
        ZoneRole = Qt::UserRole + 1,
        IdRole,
        NameRole
    };

    ZoneManager(Doc *doc, QObject *parent = nullptr);
    ~ZoneManager();

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    int count() const;

    Q_INVOKABLE Zone *zone(int index) const;
    Q_INVOKABLE Zone *zoneById(quint32 id) const;

    Q_INVOKABLE Zone *addZone(const QString &name = QString());
    Q_INVOKABLE void removeZone(int index);
    Q_INVOKABLE void moveZone(int from, int to);
    /** Move a scene to position $toIndex of a group (the same or another one), with its options */
    Q_INVOKABLE void moveScene(int fromZone, quint32 functionId, int toZone, int toIndex);

    /** Stop every function started by any zone */
    Q_INVOKABLE void stopAll();

    /** Every group with a running scene jumps to its next/previous scene */
    Q_INVOKABLE void nextAll();
    Q_INVOKABLE void previousAll();

    /** Pause every running scene; the resume plays again those it paused
     *  (not what was paused already). A new show isn't paused */
    bool paused() const;
    void setPaused(bool paused);

    /** Apply the release mode of a scene about to start in a group to all the groups */
    void releaseFor(Zone *source, quint32 fid);

    void clear();

    bool loadXML(QXmlStreamReader &root);
    bool saveXML(QXmlStreamWriter *doc) const;

signals:
    void countChanged();
    void pausedChanged();

private:
    /** IDs are never given twice in a show (deleted groups included) */
    quint32 nextZoneId() const;
    /** Stop and delete every zone (inside a model reset) */
    void deleteZones();
    void resetPause();

private:
    Doc *m_doc;
    QList<Zone *> m_zones;
    bool m_paused;
    /** The functions the pause paused */
    QSet<quint32> m_pausedFunctions;
    /** The ID of the next group: above every ID given in the show */
    quint32 m_nextZoneId;
};

#endif
