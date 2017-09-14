
import React from 'react';
import PropTypes from 'prop-types';
import { CardContent } from 'material-ui/Card';
import Collapse from 'material-ui/transitions/Collapse';
import Divider from 'material-ui/Divider';
import SectionTitle from './sectionTitle.jsx';

/**
 * This class represent the body of the resultItem.
 * It encapsulatesvarious subcomponents responsble to show for example
 * the highlighted source code, the summary of the analysis relative to this
 * this function, etc...
 */
function ResultItemBody(props) {
  return (
    <Collapse in={props.expanded} transitionDuration="auto" unmountOnExit>
      <Divider />
      <CardContent>
        <SectionTitle title="Summary" />
        <SectionTitle title="Source Code" />
      </CardContent>
    </Collapse>
  );
}

ResultItemBody.propTypes = {
  expanded: PropTypes.bool.isRequired,
};

export default ResultItemBody;
