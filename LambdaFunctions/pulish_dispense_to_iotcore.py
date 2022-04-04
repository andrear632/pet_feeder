import json
import boto3

client = boto3.client('iot-data', region_name='us-east-1')

def lambda_handler(event, context):
    response = client.publish(
        topic='topic_in',
        qos=1,
        payload=json.dumps({"message":"dispense"})
    )
    
    return {
        'statusCode': 200,
        'body': json.dumps('Published to topic')
    }