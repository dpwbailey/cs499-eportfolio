
from pymongo import MongoClient
from bson.objectid import ObjectId

class AnimalShelter(object):
    """ CRUD operations for Animal collection in MongoDB """

    def __init__(self, username='aacuser', password='password', host='nv-desktop-services.apporto.com', port=32362):
        try:
            self.client = MongoClient(f'mongodb://{username}:{password}@{host}:{port}')
            self.database = self.client['AAC']
            self.collection = self.database['animals']
        except Exception as e:
            print(f"Failed to connect to MongoDB: {e}")
            raise


    def create(self, data):
        if data:
            try:
                result = self.collection.insert_one(data)
                return result.acknowledged
            except Exception as e:
                print(f"Insert failed: {e}")
                return False
        else:
            raise ValueError("Nothing to save, because data parameter is empty")

    def read(self, query):
        try:
            return list(self.collection.find(query))
        except Exception as e:
            print(f"Query failed: {e}")
            return []
    def update(self, query, updatedInfo):
        """update documents that match the query with the updatedInfo"""
        if not updatedInfo:
            raise ValueError("no updated info provided")
        try:
            result = self.collection.update_many(query, {"$set": updatedInfo})
            return result.modified_count
        except Exception as e:
            print("failed to update: ", {e})
            return 0
    def delete(self, query):
        try:
            result = self.collection.delete_many(query)
            return result.deleted_count
        except Exception as e:
            print("Delete failed: {e}")
            return 0
