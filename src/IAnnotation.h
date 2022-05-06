#ifndef IANNOTATION_H
#define IANNOTATION_H

#include <string>

class Session;

class IAnnotation {
public:
    virtual ~IAnnotation() = default;

    virtual void list(Session *session, const std::string &tissuePath) = 0;

    virtual void save(Session *session, const std::string &tissuePath, const std::string &data) = 0;

    virtual void update(Session *session, int annotationId, const std::string &data) = 0;

    virtual void load(Session *session, int annotationId) = 0;

    virtual void remove(Session *session, int annotationId) = 0;

};

#endif //IANNOTATION_H
