import pandas as pd
from pymongo import MongoClient

client = MongoClient("mongodb://localhost:27017/")
db = client["AAC"]
collection = db["animals"]

df = pd.read_csv("aac_shelter_outcomes.csv")

records = df.to_dict(orient="records")

if records:
    collection.insert_many(records)
    print(f"Inserted {len(records)} records into AAC.animals")
else:
    print("No records found in CSV")