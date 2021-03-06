#include <memory>
#include <QMimeData>
#include "item.h"

namespace QSchematic
{
    const QString MIME_TYPE_NODE = "qschematic/node";

    class ItemMimeData : public QMimeData
    {
        Q_OBJECT
        Q_DISABLE_COPY(ItemMimeData)

    public:
        explicit ItemMimeData(std::unique_ptr<Item> item);
        virtual ~ItemMimeData() override = default;

        virtual QStringList formats() const override;
        virtual bool hasFormat(const QString& mimetype) const override;

        std::unique_ptr<Item> item() const;

    private:
        std::unique_ptr<Item> _item;
    };

}
