#ifndef ANNOTATION_H
#define ANNOTATION_H

#include "Task.h"

/**
 * @brief Annotation Request Command
 * Implemented using PostgreSQL.
 */

class Annotation : public Task, public IAnnotation
{
public:
  virtual void run(Session *session, const std::string &argument);

  /**
   * @brief Lists the annotations tied to the selected tissue.
   * The output is HTTP response, containing JSON with annotation information (id and metadata).
   *
   * @param session pointer to Session object
   * @param tissuePath relative file path to the tissue (serves as id).
   */
  virtual void list(Session *session, const std::string &tissuePath);

  /**
   * @brief Saves the annotation in the database.
   *
   * @param session pointer to Session object
   * @param tissuePath relative file path to the tissue (serves as id)
   * @param data JSON annotation data represented as a string
   */
  virtual void save(Session *session, const std::string &tissuePath, const std::string &data);

  /**
   * @brief Updates the annotation in the database.
   *
   * @param session pointer to Session object
   * @param annotationId annotation id (from the annotations table in the database)
   * @param data JSON annotation data represented as a string
   */
  virtual void update(Session *session, int annotationId, const std::string &data);

  /**
   * @brief Loads the annotation from the database.
   * The output is HTTP response, containing the annotation data in JSON format.
   *
   * @param session pointer to Session object
   * @param annotationId annotation id (from the annotations table in the database)
   */
  virtual void load(Session *session, int annotationId);

  /**
   * @brief Removes the annotation from the database.
   *
   * @param session pointer to Session object
   * @param annotationId annotation id (from the annotations table in the database)
   */
  virtual void remove(Session *session, int annotationId);
};

#endif // IANNOTATION_H