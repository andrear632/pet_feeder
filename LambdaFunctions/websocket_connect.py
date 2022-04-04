import json
import boto3

def lambda_handler(event, context):
    dynamodb = boto3.resource('dynamodb')
    table = dynamodb.Table('connections')
    response = table.put_item(
        Item={
            'conn_id': event["requestContext"]["connectionId"]
        }
    )
    return {
        'statusCode': 200,
    }
