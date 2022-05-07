#ifndef IANNOTATION_H
#define IANNOTATION_H

#include <string>

class Session;

/**
 * @brief Annotation interface. Implementations may vary.
 *
 */
class IAnnotation
{
public:
    virtual ~IAnnotation() = default;

    /**
     * @brief Lists annotations related to the specific tissue.
     *
     * @param session pointer to Session object
     * @param tissueIdentifier identifier of a tissue
     */
    virtual void list(Session *session, const std::string &tissueId) = 0;

    /**
     * @brief Saves the annotation.
     *
     * @param session pointer to Session object
     * @param tissueId identifier of a tissue
     * @param data annotation data
     */
    virtual void save(Session *session, const std::string &tissueId, const std::string &data) = 0;

    /**
     * @brief Updates the annotation.
     *
     * @param session pointer to Session object
     * @param annotationId identifier of an annotation
     * @param data annotation data
     */
    virtual void update(Session *session, int annotationId, const std::string &data) = 0;

    /**
     * @brief Loads the annotation.
     *
     * @param session pointer to Session object
     * @param annotationId identifier of an annotation
     */
    virtual void load(Session *session, int annotationId) = 0;

    /**
     * @brief Removes the annotation.
     *
     * @param session pointer to Session object
     * @param annotationId identifier of an annotation
     */
    virtual void remove(Session *session, int annotationId) = 0;
};

#endif // IANNOTATION_H
