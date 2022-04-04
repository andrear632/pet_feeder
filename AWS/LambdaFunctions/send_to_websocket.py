import json
import requests
import boto3

client=boto3.client('dynamodb')
api_client = boto3.client('apigatewaymanagementapi', endpoint_url='https://1c2u3s5olh.execute-api.us-east-1.amazonaws.com/production')

def lambda_handler(event, context):
    data = client.scan(
        TableName='connections'
    )
    
    for i in data["Items"]:
        connid=str(i["conn_id"]["S"])
        myobj = {'message': event["message"]}
        api_client.post_to_connection(ConnectionId=connid, Data=json.dumps(myobj))
    
    return {
        'statusCode': 200,
    }