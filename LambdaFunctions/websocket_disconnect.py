import json
import boto3

def lambda_handler(event, context):
    dynamodb = boto3.resource('dynamodb')
    table = dynamodb.Table('connections')
    response = table.delete_item(
        Key={
            'conn_id': event["requestContext"]["connectionId"]
        }
    )
    return {
        'statusCode': 200,
    }