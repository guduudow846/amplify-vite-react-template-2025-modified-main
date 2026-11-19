import type { Handler } from 'aws-lambda';
import { Amplify } from 'aws-amplify';
import { generateClient } from 'aws-amplify/data';
import { getAmplifyDataClientConfig } from '@aws-amplify/backend/function/runtime';
import { env } from '$amplify/env/storeDeviceShadow';
import type { Schema } from '../../data/resource';

const { resourceConfig, libraryOptions } = await getAmplifyDataClientConfig(env);
Amplify.configure(resourceConfig, libraryOptions);

const client = generateClient<Schema>({ authMode: 'iam' });

export const handler: Handler = async (event) => {
    if (!event.device_id) return;

    const { data: device, errors } = await client.models.Devices.get({
        device_id: event.device_id,
    });

    if (errors || !device) {
        console.log('GetDevice errors:', errors);
        return;
    }

    const { errors: updateErrors } = await client.models.Devices.update({
        device_id: event.device_id,
        shadow_desired: JSON.stringify(event.shadow_desired ?? {}),
        shadow_reported: JSON.stringify(event.shadow_reported ?? {}),
    });

    if (updateErrors) {
        console.log('UpdateDevice errors:', updateErrors);
    }
};