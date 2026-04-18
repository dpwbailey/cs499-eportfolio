
from pymongo import MongoClient
from bson.objectid import ObjectId

class AnimalShelter(object):
    """ CRUD operations for Animal collection in MongoDB """

    def __init__(self, username=None, password=None, host="localhost", port=27017):
        try:
            # build a connection string
            if username and password:
                connection_string = f"mongodb://{username}:{password}@{host}:{port}/"
            else:
                connection_string = f"mongodb://{host}:{port}/"

            self.client = MongoClient(connection_string)
            self.database = self.client['AAC']
            self.collection = self.database['animals']

            #indexes for common search fields
            self.collection.create_index("name")
            self.collection.create_index("animal_type")
            self.collection.create_index("breed")

        except Exception as e:
            print(f"Failed to connect to MongoDB: {e}")
            raise

    def create(self, data):
        if not data:
            raise ValueError("Nothing to save, because data parameter is empty")

        required_fields = ["animal_id", "animal_type", "name"]

        for field in required_fields:
            if field not in data or data[field] in [None, ""]:
                raise ValueError(f"Missing required field: {field}")

        try:
            # NEW: duplicate check
            existing_record = self.collection.find_one({"animal_id": data["animal_id"]})
            if existing_record:
                print(f"Insert failed: duplicate animal_id {data['animal_id']}")
                return False

            result = self.collection.insert_one(data)
            return result.acknowledged

        except Exception as e:
            print(f"Insert failed: {e}")
            return False

    def read(self, query, projection=None, limit=0):
        try:
            cursor = self.collection.find(query, projection)

            if limit > 0:
                cursor = cursor.limit(limit)

            return list(cursor)
        except Exception as e:
            print(f"Query failed: {e}")
            return []

    def update(self, query, updatedInfo):
        """update documents that match the query with the updatedInfo"""
        if not query:
            raise ValueError("update query cannot be empty")

        if not updatedInfo:
            raise ValueError("no updated info provided")

        try:
            result = self.collection.update_many(query, {"$set": updatedInfo})
            return result.modified_count
        except Exception as e:
            print(f"failed to update: {e}")
            return 0

    def delete(self, query):
        if not query:
            raise ValueError("delete query cannot be empty")

        try:
            result = self.collection.delete_many(query)
            return result.deleted_count
        except Exception as e:
            print(f"Delete failed: {e}")
            return 0
